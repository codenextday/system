LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libar230.so
LOCAL_SRC_FILES := source/Ar230dvp.c \
                   source/Ar230_tables.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include_priv \
                    $(LOCAL_PATH)/../../include_priv		\
	                $(LOCAL_PATH)/../../../include/		\
	                $(LOCAL_PATH)/../../include/
LOCAL_CFLAGS := -DLINUX -DHAL_ALTERA
LOCAL_LDLIBS := -lgpio  -lsources -lsom_ctrl
LOCAL_LDFLAGS := -L$(LOCAL_PATH)/../../../prebuildlib
LOCAL_SYSTEM_SHARED_LIBRARIES := libgpio.so libsources.so

include $(BUILD_SHARED_LIBRARY)
