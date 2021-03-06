# This file is included by the top level services directory to collect source
# files
LOCAL_REL_DIR := core/jni

LOCAL_CFLAGS += -Wall -Wno-unused-parameter -Wno-unused-function -Wformat=0

LOCAL_CFLAGS += -Wno-multichar
LOCAL_CFLAGS += -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0
LOCAL_CFLAGS += -Wno-error=format-security -Wno-unused-variable -Wno-undefined-bool-conversion

ifneq ($(ENABLE_CPUSETS),)
ifneq ($(ENABLE_SCHED_BOOST),)
LOCAL_CFLAGS += -DUSE_SCHED_BOOST
endif
endif
#debug for AFFINITY native
#LOCAL_CFLAGS += -DDEBUG_AFFINITY

LOCAL_PREBUILT_LIBS := libtinyxml:libs/libtinyxml.so

LOCAL_CLANG := true

LOCAL_SRC_FILES += \
    $(LOCAL_REL_DIR)/com_android_server_AlarmManagerService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_am_BatteryStatsService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_am_ActivityManagerService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_AssetAtlasService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_connectivity_Vpn.cpp \
    $(LOCAL_REL_DIR)/com_android_server_ConsumerIrService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_HardwarePropertiesManagerService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_hdmi_HdmiCecController.cpp \
    $(LOCAL_REL_DIR)/com_android_server_input_InputApplicationHandle.cpp \
    $(LOCAL_REL_DIR)/com_android_server_input_InputManagerService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_input_InputWindowHandle.cpp \
    $(LOCAL_REL_DIR)/com_android_server_lights_LightsService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_location_GnssLocationProvider.cpp \
    $(LOCAL_REL_DIR)/com_android_server_location_FlpHardwareProvider.cpp \
    $(LOCAL_REL_DIR)/com_android_server_power_PowerManagerService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_affinity_AffinityService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_SerialService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_SystemServer.cpp \
    $(LOCAL_REL_DIR)/com_android_server_tv_TvUinputBridge.cpp \
    $(LOCAL_REL_DIR)/com_android_server_tv_TvInputHal.cpp \
    $(LOCAL_REL_DIR)/com_android_server_vr_VrManagerService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_UsbDeviceManager.cpp \
    $(LOCAL_REL_DIR)/com_android_server_UsbMidiDevice.cpp \
    $(LOCAL_REL_DIR)/com_android_server_UsbHostManager.cpp \
    $(LOCAL_REL_DIR)/com_android_server_VibratorService.cpp \
    $(LOCAL_REL_DIR)/com_android_server_PersistentDataBlockService.cpp \
    $(LOCAL_REL_DIR)/onload.cpp

LOCAL_SRC_FILES += \
	$(LOCAL_REL_DIR)/rkdisplay/drmresources.cpp \
	$(LOCAL_REL_DIR)/rkdisplay/drmconnector.cpp \
	$(LOCAL_REL_DIR)/rkdisplay/drmcrtc.cpp \
	$(LOCAL_REL_DIR)/rkdisplay/drmencoder.cpp \
	$(LOCAL_REL_DIR)/rkdisplay/drmproperty.cpp \
	$(LOCAL_REL_DIR)/rkdisplay/drmmode.cpp \
        $(LOCAL_REL_DIR)/com_android_server_rkdisplay_RkDisplayModes.cpp \

LOCAL_SRC_FILES += \
    $(LOCAL_REL_DIR)/affinity/Process.cpp \
    $(LOCAL_REL_DIR)/affinity/configuration.cpp

LOCAL_C_INCLUDES += \
    $(LOCAL_REL_DIR)/affinity \
    $(LOCAL_REL_DIR)/affinity


LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
    frameworks/base/services \
    frameworks/base/libs \
    frameworks/base/libs/hwui \
    frameworks/base/core/jni \
    frameworks/native/services \
    libcore/include \
    libcore/include/libsuspend \
    system/security/keystore/include \
    external/libdrm \
    external/libdrm/include/drm \
    $(call include-path-for, libhardware)/hardware \
    $(call include-path-for, libhardware_legacy)/hardware_legacy \

LOCAL_SHARED_LIBRARIES += \
    libandroid_runtime \
    libandroidfw \
    libbinder \
    libcutils \
    liblog \
    libhardware \
    libhardware_legacy \
    libkeystore_binder \
    libnativehelper \
    libutils \
    libui \
    libinput \
    libinputflinger \
    libinputservice \
    libsensorservice \
    libskia \
    libgui \
    libusbhost \
    libsuspend \
    libEGL \
    libGLESv2 \
    libnetutils \
    libdrm \
    libsync \
    libtinyxml \
    libstdc++ \
    libc
