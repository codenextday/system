LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libmipi_dphy.so
LOCAL_SRC_FILES := source/mipi_dphy.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../include/include_priv		\
	                $(LOCAL_PATH)/../../../include/     \
					$(LOCAL_PATH)/../../include/
LOCAL_CFLAGS := -DLINUX -DHAL_ALTERA

include $(BUILD_SHARED_LIBRARY)
