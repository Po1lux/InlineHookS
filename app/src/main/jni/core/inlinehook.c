#include <include/shared.h>
#include "inlinehook.h"
#include "fixins.h"

bool initArm32HookInfo(HOOKINFO *hookinfo){

    if(hookinfo == NULL)
    {
        LOGI("pstInlineHook is null");
        return false;
    }
    //@cs 备份8字节
    memcpy(hookinfo->backupOpcode,hookinfo->pHookAddr,8);
    return true;
}

bool buildShellcodeStub(HOOKINFO *hookinfo){
    bool bRet = false;

    while(1)
    {
        if(hookinfo == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }

        void *p_shellcode_start_s = &_shellcode_begin_s;
        void *p_shellcode_end_s = &_shellcode_end_s;
        void *p_hookstub_function_addr_s = &_new_function_addr_s;
        void *p_old_function_addr_s = &_old_function_addr_s;

        size_t sShellCodeLength = p_shellcode_end_s - p_shellcode_start_s;
        //malloc一段内存存放shellcode
        void *pNewShellCode = malloc(sShellCodeLength);
        if(pNewShellCode == NULL)
        {
            LOGI("shell code malloc fail.");
            break;
        }
        memcpy(pNewShellCode, p_shellcode_start_s, sShellCodeLength);
        //更改stub代码页属性，改成可读可写可执行
        if(ChangePageProperty(pNewShellCode, sShellCodeLength) == false)
        {
            LOGI("change shell code page property fail.");
            break;
        }

        //设置跳转到外部stub函数去
        void **ppHookStubFunctionAddr = pNewShellCode + (p_hookstub_function_addr_s - p_shellcode_start_s);
        *ppHookStubFunctionAddr = hookinfo->onCallBack;    //@cs 更新hook函数地址

        //备份外部stub函数运行完后跳转的函数地址指针，用于填充老函数的新地址
        hookinfo->ppOldFuncAddr  = pNewShellCode + (p_old_function_addr_s - p_shellcode_start_s);

        //填充shellcode地址到hookinfo中，用于构造hook点位置的跳转指令
        hookinfo->pShellcodeAddr = pNewShellCode;

        bRet = true;
        break;
    }

    return bRet;
}

bool buildOldFuncStub(HOOKINFO *hookinfo)
{
    bool bRet = false;
    void *pFixedIns;
    int fixedLen;
    pFixedIns = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

    while(1)
    {
        if(hookinfo == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }

        void * pNewEntryForOldFunction = calloc(0x100,1);
        if(pNewEntryForOldFunction == NULL)
        {
            LOGI("new entry for old function malloc fail.");
            break;
        }

        if(ChangePageProperty(pNewEntryForOldFunction, 16) == false)
        {
            LOGI("change new entry page property fail.");
            break;
        }
        fixedLen = fixInsArm32(pFixedIns,hookinfo);


        memcpy(pNewEntryForOldFunction, pFixedIns, fixedLen);
        //填充跳转指令
        if(buildArmJumpOpcode(pNewEntryForOldFunction + fixedLen, hookinfo->pHookAddr + 8) == false)
        {
            LOGI("build jump opcodes for new entry fail.");
            break;
        }
        //填充shellcode里stub的回调地址
        *(hookinfo->ppOldFuncAddr) = pNewEntryForOldFunction;

        bRet = true;
        break;
    }

    return bRet;
}

bool rebuildHookAddrOpcode(HOOKINFO *hookinfo){
    bool bRet = false;

    while(1)
    {
        if(hookinfo == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }
        //修改原位置的页属性，保证可写
        if(ChangePageProperty(hookinfo->pHookAddr, 8) == false)
        {
            LOGI("change page property error.");
            break;
        }
        //填充跳转指令
        if(buildArmJumpOpcode(hookinfo->pHookAddr, hookinfo->pShellcodeAddr) == false)
        {
            LOGI("build jump opcodes for new entry fail.");
            break;
        }
        bRet = true;
        break;
    }

    return bRet;


}


bool ChangePageProperty(void *pAddress, size_t size)
{
    bool bRet = false;

    if(pAddress == NULL)
    {
        LOGI("change page property error.");
        return bRet;
    }

    //计算包含的页数、对齐起始地址
    unsigned long ulPageSize = (unsigned long) sysconf(_SC_PAGESIZE);
    int iProtect = PROT_READ | PROT_WRITE | PROT_EXEC;
    unsigned long ulNewPageStartAddress = (unsigned long)(pAddress) & ~(ulPageSize - 1);
    long lPageCount = (size / ulPageSize) + 1;

    long l = 0;
    while(l < lPageCount)
    {
        //利用mprotect改页属性
        int iRet = mprotect((void *)(ulNewPageStartAddress), ulPageSize, iProtect);
        if(-1 == iRet)
        {
            LOGI("mprotect error:%s", strerror(errno));
            return bRet;
        }
        l++;
    }
    return true;
}

bool buildArmJumpOpcode(void *pCurAddress , void *pJumpAddress)
{
    bool bRet = false;
    while(1)
    {
        if(pCurAddress == NULL || pJumpAddress == NULL)
        {
            LOGI("address null.");
            break;
        }
        //LDR PC, [PC, #-4]
        //addr
        //LDR PC, [PC, #-4]对应的机器码为：0xE51FF004
        //addr为要跳转的地址。该跳转指令范围为32位，对于32位系统来说即为全地址跳转。
        //缓存构造好的跳转指令（ARM下32位，两条指令只需要8个bytes）
        BYTE szLdrPCOpcodes[8] = {0x04, 0xF0, 0x1F, 0xE5};
        //将目的地址拷贝到跳转指令缓存位置
        memcpy(szLdrPCOpcodes + 4, &pJumpAddress, 4);

        //将构造好的跳转指令刷进去
        memcpy(pCurAddress, szLdrPCOpcodes, 8);
        cacheflush(*((uint32_t*)pCurAddress), 8, 0);
        bRet = true;
        break;
    }
    return bRet;
}

bool HookArm32(HOOKINFO *hookinfo)
{
    bool bRet = false;

    while(1)
    {
        if(hookinfo == NULL)
        {
            LOGI("pstInlineHook is null.");
            break;
        }

        //设置ARM下inline hook的基础信息
        //@cs 备份8字节指令
        if(initArm32HookInfo(hookinfo) == false)
        {
            LOGI("Init Arm HookInfo fail.");
            break;
        }

        //构造stub，功能是保存寄存器状态，同时跳转到目标函数，然后跳转回原函数
        //需要目标地址，返回stub地址，同时还有old指针给后续填充
        if(buildShellcodeStub(hookinfo) == false)
        {
            LOGI("BuildStub fail.");
            break;
        }

        //负责重构原函数头，功能是修复指令，构造跳转回到原地址下
        //需要原函数地址
        if(buildOldFuncStub(hookinfo) == false)
        {
            LOGI("BuildOldFunction fail.");
            break;
        }

        //负责重写原函数头，功能是实现inline hook的最后一步，改写跳转
        //需要cacheflush，防止崩溃
        if(rebuildHookAddrOpcode(hookinfo) == false)
        {
            LOGI("RebuildHookAddress fail.");
            break;
        }
        bRet = true;
        break;
    }

    return bRet;
}

void * GetModuleBaseAddr(pid_t pid, char* pszModuleName)
{
    FILE *pFileMaps = NULL;
    unsigned long ulBaseValue = 0;
    char szMapFilePath[256] = {0};
    char szFileLineBuffer[1024] = {0};

    //pid判断，确定maps文件
    if (pid < 0)
    {
        snprintf(szMapFilePath, sizeof(szMapFilePath), "/proc/self/maps");
    }
    else
    {
        snprintf(szMapFilePath, sizeof(szMapFilePath),  "/proc/%d/maps", pid);
    }

    pFileMaps = fopen(szMapFilePath, "r");
    if (NULL == pFileMaps)
    {
        return (void *)ulBaseValue;
    }

    //循环遍历maps文件，找到相应模块，截取地址信息
    while (fgets(szFileLineBuffer, sizeof(szFileLineBuffer), pFileMaps) != NULL)
    {
        if (strstr(szFileLineBuffer, pszModuleName))
        {
            char *pszModuleAddress = strtok(szFileLineBuffer, "-");
            if (pszModuleAddress)
            {
                ulBaseValue = strtoul(pszModuleAddress, NULL, 16);

                if (ulBaseValue == 0x8000)
                    ulBaseValue = 0;

                break;
            }
        }
    }
    fclose(pFileMaps);
    return (void *)ulBaseValue;
}