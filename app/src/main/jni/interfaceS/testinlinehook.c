//
// Created by seanchen on 2020-02-19.
//
#include <stdbool.h>
#include <stdint.h>
#include <asm/ptrace.h>
#include "interface.h"
#include <jni.h>
#include <android/log.h>

#define TAG "cs"
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)


void afterHook(struct pt_regs *regs) {
    regs->uregs[0] = 222;
}

void beforeHook(struct pt_regs *regs, uint32_t LRValue) {


    registerAfterHook((void *) LRValue, afterHook);

}


void mainfunc() {
    //hook native.cpp中的test函数
    char *packageName = "com.pollux.delete";
    char *soPath = "/data/app/com.pollux.inlinehooks-1/lib/arm/libnative.so";
    char *funcName = "test";

    bool res = registerHook(packageName, soPath, funcName, beforeHook, afterHook);

}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    mainfunc();

    return JNI_VERSION_1_6;
}