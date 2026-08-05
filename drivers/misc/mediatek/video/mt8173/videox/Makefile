#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

# drivers/video/mtk/Makefile
#
#


obj-$(CONFIG_MTK_FB) += mtkfb_console.o             \
			disp_assert_layer.o         \
			mtkfb.o                     \
			mtkfb_vsync.o               \
			debug.o                     \
			primary_display.o           \
			disp_lcm.o                  \
			disp_utils.o                \
			mtkfb_fence.o               \
			mtk_disp_mgr.o              \
			fbconfig_kdebug_rome.o      \
			mtk_mira.o                  \
			mtk_ovl.o                   \
			compat_mtk_disp_mgr.o

subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmp/

# Bound the primary IF_VSYNC wait so an RDMA0_DONE stall cannot latch the
# whole display pipeline into a permanent freeze. Revertible: drop this flag.
ccflags-y += -DXDPLUS_PRIM_VSYNC_TIMEOUT
# Advertise DISP_OUTPUT_CAP_SWITCHABLE in DISP_IOCTL_GET_DISPLAY_CAPS so
# the vendor hwcomposer stops refusing its mirror path. Revertible: drop this
# flag and the caps report falls back to DIRECT_LINK.
ccflags-y += -DXDPLUS_DISP_MODE_CAP_SWITCHABLE
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys   \
             -I$(srctree)/drivers/misc/mediatek/sync/      \
             -I$(srctree)/drivers/misc/mediatek/m4u/       \
             -I$(srctree)/drivers/misc/mediatek/cmdq/v2/      \
             -I$(srctree)/drivers/misc/mediatek/base/power/$(MTK_PLATFORM) \
             -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)       \
             -I$(srctree)/drivers/misc/mediatek/cmdq/v2/$(MTK_PLATFORM)      \
             -I$(srctree)/drivers/misc/mediatek/lcm/inc/      \
             -I$(srctree)/drivers/misc/mediatek/ext_disp/$(MTK_PLATFORM) \
             -I$(srctree)/drivers/staging/android/ion/

ccflags-$(CONFIG_MTK_LCM) += -I$(srctree)/drivers/misc/mediatek/lcm/inc/
ccflags-y += -I$(srctree)/drivers/staging/android/ion \
             -I$(srctree)/drivers/staging/android/ion/mtk

ifeq ($(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT),y)
#cmdq driver header need followings for secure case
ccflags-y += -I$(srctree)/include/trustzone
ccflags-y += -I$(srctree)/include
endif
ifeq ($(CONFIG_MTK_INTERNAL_HDMI_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/
endif
ifeq ($(CONFIG_MTK_INTERNAL_MHL_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mhltx/
endif
ifeq ($(CONFIG_MTK_DITHERING_SUPPORT), y)
ccflags-y += -DDITHERING_SUPPORT
endif

ifeq ($(CONFIG_MTK_GPU_SUPPORT), y)
ccflags-y += -DHWGPU_SUPPORT
endif

ifeq ($(CONFIG_MTK_ALPS_BOX_SUPPORT), y)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
else
#ccflags-y += -DMTK_DISP_IDLE_LP
endif
