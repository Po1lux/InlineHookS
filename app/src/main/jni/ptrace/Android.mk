LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CXXFLAGS  := -g -O0
LOCAL_MODULE := ptrace_main
LOCAL_SRC_FILES := ptrace_main.c
LOCAL_LDLIBS := -lm -llog
LOCAL_ARM_MODE := arm

include $(BUILD_EXECUTABLE)