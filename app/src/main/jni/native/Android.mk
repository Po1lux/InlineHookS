LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CXXFLAGS  := -g -O0
LOCAL_MODULE := native
LOCAL_SRC_FILES := native.cpp
LOCAL_LDLIBS := -lm -llog
LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)