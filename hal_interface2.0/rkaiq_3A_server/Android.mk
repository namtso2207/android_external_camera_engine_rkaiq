LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES = rkisp_3A_server.cpp
LOCAL_CPPFLAGS += -std=c++11 -Wno-error

LOCAL_CFLAGS += -Wno-error -Wno-return-type

#LOCAL_CPPFLAGS += -DLINUX
#LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

#local
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../interface \

#external/camera_engine_rkaiq
LOCAL_C_INCLUDES += \
	external/camera_engine_rkaiq \
	external/camera_engine_rkaiq/xcore \
	external/camera_engine_rkaiq/xcore/base \
	external/camera_engine_rkaiq/aiq_core \
	external/camera_engine_rkaiq/algos \
	external/camera_engine_rkaiq/hwi \
	external/camera_engine_rkaiq/iq_parser \
	external/camera_engine_rkaiq/uAPI \
	external/camera_engine_rkaiq/uAPI2 \
	external/camera_engine_rkaiq/common \
	external/camera_engine_rkaiq/include \
	external/camera_engine_rkaiq/include/iq_parser \
	external/camera_engine_rkaiq/include/uAPI \
	external/camera_engine_rkaiq/include/uAPI2 \
	external/camera_engine_rkaiq/include/xcore \
	external/camera_engine_rkaiq/include/common \
	external/camera_engine_rkaiq/include/common/mediactl \
	external/camera_engine_rkaiq/include/xcore/base \
	external/camera_engine_rkaiq/include/algos \

LOCAL_C_INCLUDES += \
	system/media/camera/include \
	frameworks/av/include \
	external/libdrm/include/drm \
	external/libdrm \
	system/core/libutils/include \
	system/core/include \
	frameworks/native/libs/binder/include

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
LOCAL_HEADER_LIBRARIES += \
       libhardware_headers \
       libbinder_headers \
       gl_headers \
       libutils_headers
else
LOCAL_C_INCLUDES += \
    hardware/libhardware/include \
    hardware/libhardware/modules/gralloc \
    system/core/include \
    system/core/include/utils \
    frameworks/av/include \
    hardware/libhardware/include
endif

LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper
LOCAL_CFLAGS += -DANDROID_VERSION_ABOVE_8_X

LOCAL_SHARED_LIBRARIES += libutils libcutils liblog

LOCAL_SHARED_LIBRARIES += \
	librkaiq \
	librkisp

LOCAL_CPPFLAGS += \
    -DUSING_METADATA_NAMESPACE=using\ ::android::hardware::camera::common::V1_0::helper::CameraMetadata

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
endif

ifeq (rk356x, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V21
endif

ifeq (rv1126, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V20
endif

ifeq (rk3588, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V30
endif

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:= rkaiq_3A_server
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := $(LOCAL_MODULE)
LOCAL_MODULE_STEM_64 := $(LOCAL_MODULE)_64

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES = rkisp_3A_server_multi_ctl.cpp

LOCAL_CPPFLAGS += -std=c++11 -Wno-error

LOCAL_CFLAGS += -Wno-error -Wno-return-type

#LOCAL_CPPFLAGS += -DLINUX
#LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

#local
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../interface \

#external/camera_engine_rkaiq
LOCAL_C_INCLUDES += \
	external/camera_engine_rkaiq \
	external/camera_engine_rkaiq/xcore \
	external/camera_engine_rkaiq/xcore/base \
	external/camera_engine_rkaiq/aiq_core \
	external/camera_engine_rkaiq/algos \
	external/camera_engine_rkaiq/hwi \
	external/camera_engine_rkaiq/iq_parser \
	external/camera_engine_rkaiq/uAPI \
	external/camera_engine_rkaiq/uAPI2 \
	external/camera_engine_rkaiq/common \
	external/camera_engine_rkaiq/include \
	external/camera_engine_rkaiq/include/iq_parser \
	external/camera_engine_rkaiq/include/uAPI \
	external/camera_engine_rkaiq/include/uAPI2 \
	external/camera_engine_rkaiq/include/xcore \
	external/camera_engine_rkaiq/include/common \
	external/camera_engine_rkaiq/include/common/mediactl \
	external/camera_engine_rkaiq/include/xcore/base \
	external/camera_engine_rkaiq/include/algos \

LOCAL_C_INCLUDES += \
	system/media/camera/include \
	frameworks/av/include \
	external/libdrm/include/drm \
	external/libdrm \
	system/core/libutils/include \
	system/core/include \
	frameworks/native/libs/binder/include

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
LOCAL_HEADER_LIBRARIES += \
       libhardware_headers \
       libbinder_headers \
       gl_headers \
       libutils_headers
else
LOCAL_C_INCLUDES += \
    hardware/libhardware/include \
    hardware/libhardware/modules/gralloc \
    system/core/include \
    system/core/include/utils \
    frameworks/av/include \
    hardware/libhardware/include
endif

LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper
LOCAL_CFLAGS += -DANDROID_VERSION_ABOVE_8_X

LOCAL_SHARED_LIBRARIES += libutils libcutils liblog

LOCAL_SHARED_LIBRARIES += \
	librkaiq \
	librkisp

LOCAL_CPPFLAGS += \
    -DUSING_METADATA_NAMESPACE=using\ ::android::hardware::camera::common::V1_0::helper::CameraMetadata

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
endif

ifeq (rk356x, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V21
endif

ifeq (rv1126, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V20
endif

ifeq (rk3588, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V30
endif

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:= rkaiq_multi_cam_3A_server
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := $(LOCAL_MODULE)
LOCAL_MODULE_STEM_64 := $(LOCAL_MODULE)_64
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES = rkisp_3A_demo.cpp

LOCAL_CPPFLAGS += -std=c++11 -Wno-error -frtti
LOCAL_CPPFLAGS += -std=c++1y
LOCAL_CPPFLAGS += -DANDROID_OS

#Namespace Declaration
LOCAL_CPPFLAGS += -DNAMESPACE_DECLARATION=namespace\ android\ {\namespace\ camera2
LOCAL_CPPFLAGS += -DNAMESPACE_DECLARATION_END=}
LOCAL_CPPFLAGS += -DUSING_DECLARED_NAMESPACE=using\ namespace\ android::camera2

#local
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../interface \
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../interface/include/ \
	$(LOCAL_PATH)/../interface/include_aiq/ \
	$(LOCAL_PATH)/../interface/xcore/ \
	$(LOCAL_PATH)/../interface/common/

#external/camera_engine_rkaiq
LOCAL_C_INCLUDES += \
	external/camera_engine_rkaiq \
	external/camera_engine_rkaiq/xcore \
	external/camera_engine_rkaiq/xcore/base \
	external/camera_engine_rkaiq/aiq_core \
	external/camera_engine_rkaiq/algos \
	external/camera_engine_rkaiq/hwi \
	external/camera_engine_rkaiq/iq_parser \
	external/camera_engine_rkaiq/uAPI \
	external/camera_engine_rkaiq/uAPI2 \
	external/camera_engine_rkaiq/common \
	external/camera_engine_rkaiq/include \
	external/camera_engine_rkaiq/include/iq_parser \
	external/camera_engine_rkaiq/include/uAPI \
	external/camera_engine_rkaiq/include/uAPI2 \
	external/camera_engine_rkaiq/include/xcore \
	external/camera_engine_rkaiq/include/common \
	external/camera_engine_rkaiq/include/common/mediactl \
	external/camera_engine_rkaiq/include/xcore/base \
	external/camera_engine_rkaiq/include/algos \

LOCAL_C_INCLUDES += \
	system/media/camera/include \
	frameworks/av/include \
	external/libdrm/include/drm \
	external/libdrm \
	system/core/libutils/include \
	system/core/include \
	frameworks/native/libs/binder/include

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
LOCAL_HEADER_LIBRARIES += \
       libhardware_headers \
       libbinder_headers \
       gl_headers \
       libutils_headers
else
LOCAL_C_INCLUDES += \
    hardware/libhardware/include \
    hardware/libhardware/modules/gralloc \
    system/core/include \
    system/core/include/utils \
    frameworks/av/include \
    hardware/libhardware/include
endif

LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper
LOCAL_CFLAGS += -DANDROID_VERSION_ABOVE_8_X
LOCAL_CFLAGS += -Wno-error -Wno-return-type
LOCAL_CFLAGS += -DANDROID_OS

LOCAL_SHARED_LIBRARIES += libutils libcutils liblog

LOCAL_SHARED_LIBRARIES += \
	librkaiq \
	librkisp

LOCAL_CPPFLAGS += \
    -DUSING_METADATA_NAMESPACE=using\ ::android::hardware::camera::common::V1_0::helper::CameraMetadata

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
endif

ifeq (rk356x, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V21
endif

ifeq (rv1126, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V20
endif

ifeq (rk3588, $(strip $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DISP_HW_V30
endif


LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:= rkaiq_3A_demo
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := $(LOCAL_MODULE)
LOCAL_MODULE_STEM_64 := $(LOCAL_MODULE)_64
include $(BUILD_EXECUTABLE)
