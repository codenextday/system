LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := example
LOCAL_SRC_FILES := example.c
LOCAL_LDLIBS = -lexlib
LOCAL_SYSTEM_SHARED_LIBRARIES = libexlib.so
include $(BUILD_EXECUTABLE)
