/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/**
* @file    mt_hotplug_strategy_platform.h
* @brief   hotplug strategy (hps) - header file for platform defines
*/

#ifndef __MT_HOTPLUG_STRATEGY_PLATFORM_H__
#define __MT_HOTPLUG_STRATEGY_PLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/sched/rt.h>		/* MAX_RT_PRIO */

/*
 * CONFIG - compile time
 */
#define HPS_TASK_PRIORITY		(MAX_RT_PRIO - 3)
#define HPS_TIMER_INTERVAL_MS		200

#define MAX_CPU_UP_TIMES		10
#define MAX_CPU_DOWN_TIMES		100
#define MAX_TLP_TIMES			10
/* cpu capability of big / little = 1.7, aka 170, 170 - 100 = 70 */
#define CPU_DMIPS_BIG_LITTLE_DIFF	70

/*
 * CONFIG - runtime
 */
#define DEF_CPU_UP_THRESHOLD		80
#define DEF_CPU_UP_TIMES		1
#define DEF_CPU_DOWN_THRESHOLD		70
#define DEF_CPU_DOWN_TIMES		10
#define DEF_TLP_TIMES			1

#define EN_CPU_INPUT_BOOST		1
#define DEF_CPU_INPUT_BOOST_CPU_NUM	2

#define EN_CPU_RUSH_BOOST		1
#define DEF_CPU_RUSH_BOOST_THRESHOLD	98
#define DEF_CPU_RUSH_BOOST_TIMES	1

/*
 * hps (MTK hotplug strategy) is disabled on this device, and has been by every
 * userspace that ever ran on it: /vendor/etc/init/hw/init.mt8173.rc turns it off
 * with the comment "it causes noticeable stutters", and device/gpd/xdplus's
 * init.xdplus.rc turns it off again at `on init`.
 *
 * Neither of those can reach the window this default governs. hps starts
 * down-migrating at a remarkably constant ~2.12 s -- it takes CPU5 offline in
 * every boot ever captured, before init has processed anything at all. Once in
 * five boots it takes CPU4 with it, and taking the last big core offline tears
 * down the A72 cluster's cpufreq policy, which is then never re-registered:
 * /sys/devices/system/cpu/cpu4/cpufreq/ does not exist for the rest of that boot
 * even though `online` still reads 0-5.
 *
 * So the platform default is only ever in force for the ~2.5 s during which it
 * can do harm and no userspace can intervene. Turning it off here closes that
 * window; the two rc writes stay as belt-and-braces for any kernel built without
 * this change.
 */
#define EN_HPS				0
#define EN_LOG_NOTICE			1
#define EN_LOG_INFO			0
#define EN_LOG_DEBUG			0

#ifdef __cplusplus
}
#endif

#endif /* __MT_HOTPLUG_STRATEGY_PLATFORM_H__ */
