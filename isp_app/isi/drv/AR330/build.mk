LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libar330.so
LOCAL_SRC_FILES := source/Ar330dvp.c \
                   source/Ar330_tables.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include_priv \
                    $(LOCAL_PATH)/../../include_priv		\
	                $(LOCAL_PATH)/../../../include/		\
	                $(LOCAL_PATH)/../../../isi/include/
LOCAL_CFLAGS := -DLINUX -DHAL_ALTERA
LOCAL_LDLIBS := -lgpio  -lsources -lcam_engine
LOCAL_SYSTEM_SHARED_LIBRARIES := libgpio.so libsources.so libcam_engine.so

include $(BUILD_SHARED_LIBRARY)
