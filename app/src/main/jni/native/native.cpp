//
// Created by seanchen on 2020-02-12.
//

#include "com_pollux_inlinehooks_MainActivity.h"

extern "C"
int test(int a,int b);

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
    int a = test(2,4);
    LOGI("aaaaa:%d", a);
}

extern "C"
int test(int a,int b){
    int m= 1;
    int n = 0x10;
    int res = a*b;
    return res+m+n;
}