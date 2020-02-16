LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CXXFLAGS  := -g -O0
LOCAL_MODULE := inlinehook
LOCAL_SRC_FILES := inlinehook.c shellcodestub.s fixins.c utils.c
LOCAL_LDLIBS := -lm -llog
LOCAL_ARM_MODE := arm
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

include $(BUILD_STATIC_LIBRARY)