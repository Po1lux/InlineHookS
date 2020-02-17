//
// Created by seanchen on 2020-02-14.
//

#include <stdlib.h>
#include <android/log.h>
#define BYTE uint8_t
#define OPCODEMAXLEN 8
#define TAG "cs"
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

typedef struct sHookInfo{
    void *pHookAddr;
    void *pShellcodeAddr;
    void (* onCallBack)(struct pt_regs *);

//    void (* beforeHook)(struct pt_regs *);
//    void (* afterHook)(struct pt_regs *);
    void **ppOldFuncAddr;
    BYTE backupOpcode[OPCODEMAXLEN];
}HOOKINFO;