LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libgpio.so
LOCAL_SRC_FILES := source/gpio.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include/ \
	                $(LOCAL_PATH)/../../../include/		\
	                $(LOCAL_PATH)/../../../include/ebase/	\
	                $(LOCAL_PATH)/../../../include/oslayer/

LOCAL_CFLAGS := -DLINUX -DHAL_ALTERA

include $(BUILD_SHARED_LIBRARY)
