// SPDX-License-Identifier: GPL-2.0
/*
 * devfreq bridge for the MediaTek mt_gpufreq GPU DVFS driver.
 *
 * The MT8176 GPU (PowerVR GX6250, driven by the RGX DDK which calls
 * mt_gpufreq_target() directly) never registered with the devfreq
 * framework, so /sys/class/devfreq stayed empty and userspace hardware
 * monitors (kernel-manager apps etc.) report the GPU frequency, governor
 * and OPP table as "Unknown"/"N/A".
 *
 * This shim registers a devfreq device named "mt8176-gpu" on top of
 * mt_gpufreq.  The default governor is "userspace", which never changes
 * the frequency on its own: the RGX DDK's internal DVFS loop keeps
 * driving the GPU exactly as before, and the devfreq node only reports
 * live state until a userspace tool explicitly writes target_freq /
 * min_freq / max_freq / governor.
 *
 * Units: GPU devfreq drivers (kgsl, mali) report Hz, and userspace tools
 * assume Hz here, so this shim converts: mt_gpufreq natively uses kHz.
 */

#include <linux/devfreq.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "mt_gpufreq.h"

/* Ascending Hz copy of the runtime OPP table (mt_gpufreq is idx 0 = max). */
static unsigned int *xdplus_gpu_freq_table;
static unsigned int xdplus_gpu_num_states;

static int xdplus_gpu_devfreq_target(struct device *dev,
				     unsigned long *freq, u32 flags)
{
	int i, best = -1;

	if (!mt_gpufreq_dvfs_ready() || !xdplus_gpu_num_states)
		return -ENODEV;

	/* lowest OPP that is >= *freq ("lowest-upper-than-freq" rule) */
	for (i = 0; i < xdplus_gpu_num_states; i++) {
		if (xdplus_gpu_freq_table[i] >= *freq) {
			best = i;
			break;
		}
	}
	if (best < 0)
		best = xdplus_gpu_num_states - 1; /* clamp to maximum */

	/* map ascending table index back to mt_gpufreq's 0 = highest */
	mt_gpufreq_target((unsigned int)(xdplus_gpu_num_states - 1 - best));

	*freq = (unsigned long)mt_gpufreq_get_cur_freq() * 1000UL;
	return 0;
}

static int xdplus_gpu_devfreq_get_cur_freq(struct device *dev,
					   unsigned long *freq)
{
	*freq = (unsigned long)mt_gpufreq_get_cur_freq() * 1000UL;
	return 0;
}

static struct devfreq_dev_profile xdplus_gpu_devfreq_profile = {
	.polling_ms = 0, /* no polling: userspace governor, live cur_freq */
	.target = xdplus_gpu_devfreq_target,
	.get_cur_freq = xdplus_gpu_devfreq_get_cur_freq,
};

static struct platform_device *xdplus_gpu_devfreq_pdev;
static struct devfreq *xdplus_gpu_devfreq;

static int __init xdplus_gpufreq_devfreq_init(void)
{
	int i, n;

	if (!mt_gpufreq_dvfs_ready()) {
		pr_warn("xdplus-gpufreq-devfreq: GPU DVFS not ready, not registering\n");
		return -ENODEV;
	}

	n = mt_gpufreq_get_dvfs_table_num();
	if (n <= 0) {
		pr_warn("xdplus-gpufreq-devfreq: empty GPU OPP table\n");
		return -ENODEV;
	}

	xdplus_gpu_freq_table = kcalloc(n, sizeof(unsigned int), GFP_KERNEL);
	if (!xdplus_gpu_freq_table)
		return -ENOMEM;

	for (i = 0; i < n; i++) {
		unsigned int khz = mt_gpufreq_get_freq_by_idx(i);

		if (khz == 0 || khz > GPU_DVFS_MAX_FREQ) {
			pr_warn("xdplus-gpufreq-devfreq: bad OPP %d kHz at idx %d\n",
				khz, i);
			kfree(xdplus_gpu_freq_table);
			xdplus_gpu_freq_table = NULL;
			return -ENODEV;
		}
		/* Hz, ascending order (mt_gpufreq table is idx 0 = highest) */
		xdplus_gpu_freq_table[n - 1 - i] = (unsigned int)khz * 1000U;
	}
	xdplus_gpu_num_states = n;

	xdplus_gpu_devfreq_pdev =
		platform_device_register_simple("mt8176-gpu", PLATFORM_DEVID_NONE,
						NULL, 0);
	if (IS_ERR(xdplus_gpu_devfreq_pdev)) {
		kfree(xdplus_gpu_freq_table);
		xdplus_gpu_freq_table = NULL;
		return PTR_ERR(xdplus_gpu_devfreq_pdev);
	}

	xdplus_gpu_devfreq_profile.initial_freq =
		(unsigned long)mt_gpufreq_get_cur_freq() * 1000UL;
	xdplus_gpu_devfreq_profile.freq_table = xdplus_gpu_freq_table;
	xdplus_gpu_devfreq_profile.max_state = xdplus_gpu_num_states;

	xdplus_gpu_devfreq = devfreq_add_device(&xdplus_gpu_devfreq_pdev->dev,
						&xdplus_gpu_devfreq_profile,
						"userspace", NULL);
	if (IS_ERR(xdplus_gpu_devfreq)) {
		platform_device_unregister(xdplus_gpu_devfreq_pdev);
		kfree(xdplus_gpu_freq_table);
		xdplus_gpu_freq_table = NULL;
		return PTR_ERR(xdplus_gpu_devfreq);
	}

	/* Give the min_freq/max_freq policy nodes meaningful defaults
	 * (they default to 0) so userspace tools display the real OPP range
	 * and min/max writes are clamped to it.
	 */
	mutex_lock(&xdplus_gpu_devfreq->lock);
	xdplus_gpu_devfreq->min_freq = xdplus_gpu_freq_table[0];
	xdplus_gpu_devfreq->max_freq = xdplus_gpu_freq_table[n - 1];
	mutex_unlock(&xdplus_gpu_devfreq->lock);

	pr_info("xdplus-gpufreq-devfreq: registered mt8176-gpu (%d OPPs, %u-%u Hz)\n",
		n, xdplus_gpu_freq_table[0], xdplus_gpu_freq_table[n - 1]);
	return 0;
}
/* mt_gpufreq_init() and the devfreq governors are module_init
 * (device_initcall); run after both so the table and "userspace" exist.
 */
late_initcall(xdplus_gpufreq_devfreq_init);
