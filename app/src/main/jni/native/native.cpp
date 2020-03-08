//
// Created by seanchen on 2020-02-12.
//

#include <zconf.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "com_pollux_inlinehooks_MainActivity.h"

#define TAG "cs"

#include <android/log.h>
#include <stdlib.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <dirent.h>

#define CPSR_T_MASK        ( 1u << 5 )
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
//void testinit() __attribute__((constructor));
extern "C"
int test(int a,int b);



extern "C"
JNIEXPORT void JNICALL Java_com_pollux_inlinehooks_MainActivity_ptracetest
        (JNIEnv *env, jclass clazz) {

    pid_t target_pid;
}











/**
 * 指令级hook测试样例，寄存器修改 uTimeCounter 的值大于0x20
 */
static unsigned int uTimeCounter = 0x1;
extern "C"
JNIEXPORT jstring JNICALL Java_com_pollux_inlinehooks_MainActivity_getTikNum
(JNIEnv *env, jclass jclazz){
    unsigned int localVar = 0x1;
    uTimeCounter +=localVar;
    LOGI("uTimeCounter:%d",uTimeCounter);
    if(uTimeCounter>=0x20){
        return env->NewStringUTF("Congradulation!");
    } else{
        return env->NewStringUTF("not enough");
    }
}

/**
 * 函数级hook测试样例，hook test 函数的返回值
 */
extern "C"
JNIEXPORT void JNICALL Java_com_pollux_inlinehooks_MainActivity_hookRet
        (JNIEnv *env, jclass jclazz){
    int a = test(2, 4);//25
    LOGI("result: %d", a);
}


extern "C"
int test(int a,int b){
    int m= 1;
    int n = 0x10;
    int res = a*b;
    return res+m+n;
}


void testinit() {
    FILE *statusFile;
    char buf[0x100];
    char name[0x20];
    char *target = "com.pollux.delete";
    if ((statusFile = fopen("/proc/self/status", "r")) == NULL) {
        LOGI("open maps error");
        return;
    }
    if (fgets(buf, sizeof(buf), statusFile)) {
        strncpy(name, buf + 6, 15);
        //LOGI("get process name: %s",name);
        char path[] = "/data/initarray/";
        strcat(path, name);
        int fd = open(path, O_CREAT, S_IRWXU);
        if (fd == -1) {
            LOGI("Message : %s", strerror(errno));
        }
//        if(strstr(target,name)){
//            LOGI("get target %s",);
//        }

    }


}