//
// Created by seanchen on 2020-02-12.
//

#include "com_pollux_inlinehooks_MainActivity.h"

extern "C"
int test(int a,int b);
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

extern "C"
JNIEXPORT void JNICALL Java_com_pollux_inlinehooks_MainActivity_hookRet
        (JNIEnv *env, jclass jclazz){
    int a = test(2,4);
    LOGI("value a: %d",a);
}

extern "C"
int test(int a,int b){
    int m= 1;
    int n = 0x10;
    int res = a*b;
    return res+m+n;
}