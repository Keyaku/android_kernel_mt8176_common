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


ifeq ($(CONFIG_MTK_HDMI_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/ext_disp/  \
             -I$(srctree)/drivers/misc/mediatek/video/     \
             -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/ \
             -I$(srctree)/drivers/misc/mediatek/sync/ \
             -I$(srctree)/drivers/misc/mediatek/mhl/driver/mhl_tx_drv/ \
             -I$(srctree)/drivers/misc/mediatek/cmdq/v2/ \
             -I$(srctree)/drivers/misc/mediatek/cmdq/v2/$(MTK_PLATFORM)/ \
             -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc/\
             -I$(srctree)/drivers/misc/mediatek/hdmi/mhltx/$(CONFIG_MTK_PLATFORM)/\
             -I$(srctree)/drivers/staging/android/ion/ \
             -I$(srctree)/drivers/staging/android/ion/mtk/ \
             -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/ \
             -I$(srctree)/drivers/misc/mediatek/lcm/inc/ \
             -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)/ \
             -I$(srctree)/drivers/misc/mediatek/mmp/ \

ifeq ($(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT),y)
#cmdq driver header need followings for secure case
ccflags-y += -I$(srctree)/include/trustzone
ccflags-y += -I$(srctree)/include
endif

ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),nxp_tda19989)
	ccflags-y +=	-DHDMI_TDA19989
endif

ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),Sii8338)
	ccflags-y +=	-DMHL_SII8338
endif

ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),Sii8348)
	ccflags-y +=	-DMHL_SII8348
endif

ifeq ($(CONFIG_MTK_ALPS_BOX_SUPPORT), y)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
ccflags-y += -DHDMI_SUB_PATH
ccflags-y += -DHDMI_SUB_PATH_PROB
ccflags-y += -DHDMI_SUB_PATH_PRESENT_FENCE_SUPPORT
endif

# Present-fence machinery only, without the box-only HDMI_SUB_PATH rework: the
# full define also flips _should_start_path()/_should_trigger_path() and moves the
# rdma-config IRQ from RDMA1/RDMA2 to OVL1/WDMA1, which would break the direct-link
# path this device uses. The external session needs a real present fence or the
# composer triggers it with fd -1 and the sub_disp path never starts.
ccflags-y += -DXDPLUS_EXTD_PRESENT_FENCE

# Source the external DISP_PATH_EVENT_IF_VSYNC from DPI0's own VSYNC interrupt
# instead of DDP_IRQ_RDMA1_DONE. RDMA1 only raises DONE while it is streaming and
# goes idle on this direct-couple path, so the stock mapping leaves the composer
# waiting out dpmgr_wait_event_timeout() on every frame ("wait VSYNC timeout on
# scenario sub_disp") and presenting at the timeout rate instead of 60 Hz. DPI0
# is the engine actually driving the HDMI scanout and ticks regardless.
ccflags-y += -DXDPLUS_EXTD_VSYNC_FROM_DPI

# Temporary §98 fix-2 instrumentation: log ext_disp_trigger() entry state
# (rate-limited to 1/s). Remove once the external path streams.
ccflags-y += -DXDPLUS_TRIGGER_PROBE

obj-y	:= extd_ddp.o extd_debug.o  extd_drv.o extd_utils.o extd_lcm.o extd_fence.o

else
obj-y := dummy.o
endif
