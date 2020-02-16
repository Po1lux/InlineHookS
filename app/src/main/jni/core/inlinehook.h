//#ifndef __arm__
//#define __arm__
//#endif __arm__
#ifndef INLINEHOOKS_H
#define INLINEHOOKS_H

#include <android/log.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arm-linux-androideabi/asm/ptrace.h>
#include <unistd.h> //cacheflush
//# include <asm/cachectl.h>

extern unsigned long _shellcode_begin_s;
extern unsigned long _shellcode_end_s;
extern unsigned long _new_function_addr_s;
extern unsigned long _old_function_addr_s;

bool initArm32HookInfo(HOOKINFO *hookinfo);
bool buildShellcodeStub(HOOKINFO *hookinfo);
bool buildOldFuncStub(HOOKINFO *hookinfo);
bool rebuildHookAddrOpcode(HOOKINFO *hookinfo);
bool ChangePageProperty(void *pAddress, size_t size);
bool buildArmJumpOpcode(void *pCurAddress , void *pJumpAddress);
bool HookArm32(HOOKINFO *hookinfo);
void * GetModuleBaseAddr(pid_t pid, char* pszModuleName);
int fixInsArm32(void *fixedIns,HOOKINFO *hookinfo);

#endif //INLINEHOOKS_H