/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_pollux_inlinehooks_MainActivity */

#ifndef _Included_com_pollux_inlinehooks_MainActivity
#define _Included_com_pollux_inlinehooks_MainActivity
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_pollux_inlinehooks_MainActivity
 * Method:    getTikNum
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL
Java_com_pollux_inlinehooks_MainActivity_getTikNum
  (JNIEnv *, jclass);

extern "C"
JNIEXPORT void JNICALL
Java_com_pollux_inlinehooks_MainActivity_hookRet
        (JNIEnv *env, jclass jclazz);

extern "C"
JNIEXPORT void JNICALL
Java_com_pollux_inlinehooks_MainActivity_ptracetest(JNIEnv *env, jclass clazz);

extern "C"
int test(int a,int b);
#ifdef __cplusplus
}
#endif
#endif
