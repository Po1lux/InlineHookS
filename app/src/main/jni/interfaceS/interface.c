//
// Created by seanchen on 2020-02-12.
//
#include "include/shared.h"
#include "inlinehook.h"
#include <jni.h>
#include <dlfcn.h>
#include "utils.h"
bool doInlineHook(void *pHookAddr, void (*onCallBack)(struct pt_regs *))
{
    bool bRet = false;

    if(pHookAddr == NULL || onCallBack == NULL)
    {
        return bRet;
    }

    HOOKINFO *hookinfo = (HOOKINFO *)malloc(sizeof(HOOKINFO));
    hookinfo->pHookAddr = pHookAddr;
    hookinfo->onCallBack = onCallBack;

    //DEMO只很对ARM指令进行演示，更通用这里需要判断区分THUMB等指令
    if(HookArm32(hookinfo) == false)
    {
        LOGI("HookArm fail.");
        free(hookinfo);
        return bRet;
    }
    //gs_vecInlineHookInfo.push_back(pstInlineHook);
    return true;
}

int EvilHookStubFunctionForIBored(struct pt_regs *regs)
{
    LOGI("In Evil Hook Stub.");
    //regs->uregs[0] = 0x333;
    return 222;
}


//-------------------------------------------------------
//-------------------------------------------------------
void test(){
    void *handle;
    int (*func)(int);
    handle = dlopen("libinterface.so",RTLD_LAZY);
    void *addr = __builtin_return_address(0);
    func = (int (*)(int))dlsym(handle,"symInt");
    //call func
    int b = (*func)(3);
    LOGI("test func: %d",b);
    dlclose(handle);

}
//-------------------------------------------------------
//-------------------------------------------------------


void ModifyIBored()
{
    LOGI("In IHook's ModifyIBored.");

    void* pModuleBaseAddr = GetModuleBaseAddr(-1, "libnative.so");
    if(pModuleBaseAddr == 0)
    {
        LOGI("geuiHookAddrt module base error.");
        return;
    }

//    uint32_t uiHookAddr = (uint32_t)pModuleBaseAddr + 0x838;
    uint32_t uiHookAddr = (uint32_t)pModuleBaseAddr + 0x810;
    LOGI("uiHookAddr is %X", uiHookAddr);
    //doInlineHook((void*)(uiHookAddr), EvilHookStubFunctionForIBored);

    void *ctx = my_dlopen("/data/app/com.pollux.inlinehooks-1/lib/arm/libnative.so");
    void *ptr = my_dlsym(ctx,"test");
    doInlineHook(ptr, EvilHookStubFunctionForIBored);
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){

    ModifyIBored();
    LOGI("jin_onload");
    return JNI_VERSION_1_6;
}
