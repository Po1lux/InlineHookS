//
// Created by seanchen on 2020-02-12.
//
#include "include/shared.h"
#include "inlinehook.h"
#include "interface.h"
#include <dlfcn.h>
#include <fcntl.h>
#include "utils.h"

//-------------------------------------------------------
//-------------------------------------------------------
//void xxxxxxxxx() {
//    void *handle;
//    int (*func)(int);
//    handle = dlopen("libinterface.so", RTLD_LAZY);
//    void *addr = __builtin_return_address(0);
//    func = (int (*)(int)) dlsym(handle, "symInt");
//    //call func
//    int b = (*func)(3);
//    LOGI("test func: %d", b);
//    dlclose(handle);
//
//}
//-------------------------------------------------------
//-------------------------------------------------------

//void EvilHookStubFunctionForIBored(struct pt_regs *regs, uint32_t LRValue) {
//    LOGI("In Evil Hook Stub.");
//    //regs->uregs[0] = 0x333;
//    int r0 = regs->uregs[0];
//    int r1 = regs->uregs[1];
//    GLRAddr = LRValue;
//    LOGI("lr addr:%u", GLRAddr);
//    doInlineHook((void *) GLRAddr, afterHooks);
//}


//
//void ModifyIBored() {
//    LOGI("In IHook's ModifyIBored.");
//
//    void *pModuleBaseAddr = GetModuleBaseAddr(-1, "libnative.so");
//    if (pModuleBaseAddr == 0) {
//        LOGI("geuiHookAddrt module base error.");
//        return;
//    }
//
//    uint32_t uiHookAddr = (uint32_t)pModuleBaseAddr + 0x838;
//    uint32_t uiHookAddr = (uint32_t) pModuleBaseAddr + 0xb8c;
//    LOGI("uiHookAddr is %X", uiHookAddr);
//    //doInlineHook((void*)(uiHookAddr), EvilHookStubFunctionForIBored);
//
//    //void *ctx = my_dlopen("/data/app/com.pollux.inlinehooks-1/lib/arm/libnative.so");
//    //void *ptr = my_dlsym(ctx,"test");
//    doInlineHook((void *) uiHookAddr, EvilHookStubFunctionForIBored);
//
//}


bool doInlineHook(void *pHookAddr, void (*onCallBack)(struct pt_regs *)) {
    bool bRet = false;

    if (pHookAddr == NULL || onCallBack == NULL) {
        return bRet;
    }

    HOOKINFO *hookinfo = (HOOKINFO *) malloc(sizeof(HOOKINFO));
    hookinfo->pHookAddr = pHookAddr;
    hookinfo->onCallBack = onCallBack;

    //DEMO只很对ARM指令进行演示，更通用这里需要判断区分THUMB等指令
    if (HookArm32(hookinfo) == false) {
        LOGI("HookArm fail.");
        free(hookinfo);
        return bRet;
    }
    //gs_vecInlineHookInfo.push_back(pstInlineHook);
    return true;
}


bool registerHook(char *packageName,
                  char *soPath,
                  char *funcName,
                  void(*beforeHook)(struct pt_regs *),
                  void(*afterHook)(struct pt_regs *)) {

    //bool istarget = initialHook(packageName);
    //void *ctx = my_dlopen("/data/app/com.pollux.inlinehooks-1/lib/arm/libnative.so");
    //void *ptr = my_dlsym(ctx, funcName);

    //--------------test
    void *pModuleBaseAddr = GetModuleBaseAddr(-1, "libnative.so");
    uint32_t uiHookAddr = (uint32_t) pModuleBaseAddr + 0xb8c;
    //-------------
    doInlineHook((void *) uiHookAddr, beforeHook);
}

bool initialHook(char *packageName) {
    FILE *statusFile;
    char buf[0x100];
    char name[0x20];
    char *target = "com.pollux.delete";
    if ((statusFile = fopen("/proc/self/status", "r")) == NULL) {
        LOGI("open maps error");
        return false;
    }
    while (fgets(buf, sizeof(buf), statusFile)) {
        strncpy(name, buf + 6, 15);
        LOGI("get process name: %s", name);
        if (strstr(target, name)) {
            LOGI("get target %s", target);
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void registerAfterHook(void *retAddr, void(*afterHook)(struct pt_regs *regs)) {
    doInlineHook(retAddr, afterHook);
}