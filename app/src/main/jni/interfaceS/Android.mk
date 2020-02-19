LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_STATIC_LIBRARIES  := inlinehook
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../core

LOCAL_MODULE    := interface
LOCAL_SRC_FILES := interface.c testinlinehook.c
LOCAL_CPPFLAGS  := -g -O0
LOCAL_LDLIBS    := -lm -llog
LOCAL_ARM_MODE  := arm

include $(BUILD_SHARED_LIBRARY)