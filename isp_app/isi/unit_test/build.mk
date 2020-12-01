LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := unit_test
LOCAL_SRC_FILES := runner/unitTestRunner.c   \
				   tests/mock/modulexy_mock.c \
				   tests/isi_ut.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include  \
                    $(LOCAL_PATH)/../include  \
					$(LOCAL_PATH)/../include_priv
LOCAL_CFLAGS := -DLINUX -DHAL_ALTERA
LOCAL_LDLIBS := -lgpio  -lsources -lsom_ctrl -lQtCore -ldl -lembUnit
LOCAL_LDFLAGS := -L$(LOCAL_PATH)/../../prebuildlib
LOCAL_SYSTEM_SHARED_LIBRARIES := libgpio.so libsources.so

include $(BUILD_EXECUTABLE)
