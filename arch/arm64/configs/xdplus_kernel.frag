# xdplus kernel config deltas over mt8176_defconfig.
# Merged via scripts/kconfig/merge_config.sh (set KFRAG=this file for kbuild.sh).
#
# Unlock 1 — input lag: Android task-profile CPU-affinity.
# Baseline has CGROUPS + CGROUP_CPUACCT + CGROUP_SCHED only.
CONFIG_CPUSETS=y
CONFIG_PROC_PID_CPUSET=y
# NOTE: CONFIG_CGROUP_SCHEDTUNE (EAS boost) is NOT available in this 2019
# CleanROM 3.18 tree — no SchedTune/EAS backport exists (grep: zero hits for
# schedtune/sched_tune anywhere). Enabling it would require backporting the
# whole EAS scheduler (heavy, boot-risk). CPUSETS above delivers the primary
# task-profile CPU-affinity fix; EAS boost is deferred to the mainline path.
# CONFIG_CGROUP_SCHEDTUNE=y  # unavailable, see above
#
# ALS/auto-brightness (LTR303): DISABLED — the chip is not populated on this
# board. The board dts declares an LTR303 at i2c9/0x29, but a live userspace
# bus scan (i2cdetect over every sensor bus, /dev/i2c-* exposed by
# CONFIG_I2C_CHARDEV below) finds nothing answering at 0x29, nor at the 0x49
# that cust_i2c.dtsi declares under &i2c2; every other declared client on those
# buses either ACKs or already has a driver bound. The dts node is design
# intent, not proof of population.
#
# Leaving the driver enabled is not free: ltr303_init_client() busy-waits
# mdelay(PON_DELAY) = 600 ms before its first register read, unconditionally,
# and that runs inside the alsps_init initcall on every boot — measured at
# 629 ms of kernel boot, the single largest initcall on this device. The ALSPS
# core stays enabled (nothing else depends on it, and it costs nothing once no
# chip driver registers); only the LTR303 chip driver goes.
CONFIG_CUSTOM_KERNEL_ALSPS=y
# CONFIG_MTK_LTR303 is not set
#
# Boot-critical (Android 11 Treble): the 2019 CleanROM binder is single-device
# (only /dev/binder) so hwservicemanager/vndservicemanager can't open their
# nodes -> InitFatalReboot. Backported the AOSP multi-/dev-instance binder into
# drivers/staging/android/binder.c; this selects the three-device default.
CONFIG_ANDROID_BINDER_DEVICES="binder,hwbinder,vndbinder"
#
# Unlock 3 - PowerVR DDK 1.9@4893595 KM: gpu_rgx/ imported
# from the ALLDOCUBE X kernel (third_party/lineageos_kernel_cube_u1005) —
# matches the vendor UM blobs + rgx.fw.signed exactly. The gpu/Makefile
# dispatches obj-y += gpu_$(word 1,MTK_GPU_VERSION)/; word 2 (clyde) selects
# m1.9ED4893595 inside gpu_rgx/Makefile. Baseline defconfig has
# CONFIG_MTK_GPU_VERSION unset (falls back to mt8173/ = old DDK 1.7).
CONFIG_MTK_GPU_VERSION="rgx clyde 1.9ED"
# DDK 1.9 + display fences use the OLD staging sync framework (baseline
# already has CONFIG_SYNC/SW_SYNC/SW_SYNC_USER=y). MTK_SYNC pinned here —
# Lesson: verify it survives into the merged .config.
CONFIG_MTK_SYNC=y
#
# Expose /dev/i2c-* so userspace i2cdetect/i2cget (already shipped in
# /system/bin) can live-scan the sensor buses; the baseline has no i2c-dev nodes
# at all. Added as an ALS diagnostic and it answered that question (see above),
# but kept: a live bus map is the fastest way to tell a declared-but-absent chip
# from a driver that fails to bind, and it costs nothing at runtime.
#
# Reading the scan: an address shows UU only when a driver is bound and
# i2cdetect skips probing it, so a client that is declared in the dts but has no
# driver bound and reads "--" is a genuine NAK, not a skip. Cross-check against
# /sys/bus/i2c/devices/*/driver.
CONFIG_I2C_CHARDEV=y
