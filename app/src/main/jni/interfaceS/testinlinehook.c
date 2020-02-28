//
// Created by seanchen on 2020-02-19.
//
//使用接口文件，直接调用interface.h中声明的函数

#include <stdbool.h>
#include <stdint.h>
#include <asm/ptrace.h>
#include "interface.h"
#include <jni.h>
#include <android/log.h>

#define TAG "cs"
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

//修改返回值为222
void afterHook(struct pt_regs *regs) {
    regs->uregs[0] = 222;
}

void beforeHook(struct pt_regs *regs, uint32_t LRValue) {


    //实现修改返回值的函数，若想查看或修改函数返回值，这段代码不能删除
    registerAfterHook((void *) LRValue, afterHook);
    //-------------------------------------------------
}

/**
 * Hook总入口
 */
void mainfunc() {
    //hook native.cpp中的test函数
    char *packageName = "com.pollux.delete";
    //TODO: soPath存在版本号如，xxx-1，如何自动定位so文件，而不是填充目录？
    char *soPath = "/data/app/com.pollux.inlinehooks-2/lib/arm/libnative.so";
    char *funcName = "test";

    bool res = registerHook(packageName, soPath, funcName, beforeHook, afterHook);

}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    mainfunc();

    return JNI_VERSION_1_6;
}