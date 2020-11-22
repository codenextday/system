LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libsources.so
LOCAL_SRC_FILES := isi.c \
                   isisup.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include_priv \
	                $(LOCAL_PATH)/../../include/ \
	                $(LOCAL_PATH)/../include/ \
	                $(LOCAL_PATH)/../include_priv/
LOCAL_CFLAGS := -DLINUX -DHAL_ALTERA
LOCAL_LDLIBS := -lsom_ctrl
LOCAL_LDFLAGS := -L$(LOCAL_PATH)/../../prebuildlib
include $(BUILD_SHARED_LIBRARY)
