LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := example_app
LOCAL_SRC_FILES := template_example_app.c

# LOCAL_C_INCLUDES := 
# LOCAL_CFLAGS := -DLINUX -DHAL_ALTERA
# LOCAL_LDLIBS := -lgpio  -lsources -lsom_ctrl
# LOCAL_LDFLAGS := -L$(LOCAL_PATH)/../../../prebuildlib
# LOCAL_SYSTEM_SHARED_LIBRARIES := libgpio.so libsources.so

include $(BUILD_EXECUTABLE)
