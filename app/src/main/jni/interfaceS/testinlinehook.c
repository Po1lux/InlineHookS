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
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define TAG "cs"
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)


/**
 * 修改返回值为222
 * @param regs
 */
void afterHook(struct pt_regs *regs) {
    LOGI("before hook result: %d", regs->uregs[0]);
    regs->uregs[0] = 222;
}

/**
 * 查看参数
 * @param regs
 * @param LRValue
 */
void beforeHook(struct pt_regs *args, uint32_t LRValue) {

    uint32_t arg0 = args->uregs[0];
    uint32_t arg1 = args->uregs[1];
    LOGI("arg0: %d  arg1: %d", arg0, arg1);

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

void ptracemaintest() {
//    char path[] = "/data/initarray/";
//    strcat(path, "ptrace");
//    int fd = open(path, O_CREAT, S_IRWXU);
//    if (fd == -1) {
//        LOGI("Message : %s", strerror(errno));
//    }
    LOGI("ptracemaintestptracemaintest");
}

//JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
//    //mainfunc();
//    ptracemaintest();
//    return JNI_VERSION_1_6;
//}
