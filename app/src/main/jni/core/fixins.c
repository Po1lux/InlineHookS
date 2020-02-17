//
// Created by seanchen on 2020-02-13.
//

#include <include/shared.h>
#include <string.h>
#include "fixins.h"

enum INSTRUCTION_TYPE {
    LDR2_ARM,   //LDR Rd,[PC,#imme]

    // BLX <label>
            BLX1_ARM,    //BLX imme32
    BLX2_ARM,    //BLX PC
    // BL <label>
            BL_ARM,
    // B <label>
            B_ARM,

    // <Add by GToad>
    // B <label>
            BEQ_ARM,
    // B <label>
            BNE_ARM,
    // B <label>
            BCS_ARM,
    // B <label>
            BCC_ARM,
    // B <label>
            BMI_ARM,
    // B <label>
            BPL_ARM,
    // B <label>
            BVS_ARM,
    // B <label>
            BVC_ARM,
    // B <label>
            BHI_ARM,
    // B <label>
            BLS_ARM,
    // B <label>
            BGE_ARM,
    // B <label>
            BLT_ARM,
    // B <label>
            BGT_ARM,
    // B <label>
            BLE_ARM,
    // </Add by GToad>

    // BX PC
            BX_ARM,

    ADD1_ARM,   //ADD Rd,PC,Rm
    ADD2_ARM,   //ADD Rd,PC
    // ADR Rd, <label>
            ADR1_ARM,
    // ADR Rd, <label>
            ADR2_ARM,
    // MOV Rd, PC
            MOV_ARM,
    // LDR Rt, <label>
            LDR1_ARM,   //LDR Rd,[PC,Rm]

    UNDEFINE,
};

static int getInsTypeArm32(uint32_t ins) {
    if (ins & 0xFF000000 == 0xEA000000) return B_ARM;   //B imme24
    if (ins & 0xFF000000 == 0xEB000000) return BL_ARM;  //BL imme24
    if (ins & 0xFFFFFFFF == 0xE12FFF1F) return BX_ARM;  //BX PC
    if (ins & 0xFF000000 == 0xFA000000) return BLX1_ARM;  //BLX imme32
    //if (ins & 0xFFFFFFFF == 0xE12FFF3F) return BLX2_ARM;  //BLX PC,未发现这种情况

    if ((ins & 0xFFFF0000) == 0xE08F0000) return ADD1_ARM;  //ADD Rd,PC,Rm
    if ((ins & 0xFFF0000F) == 0xE080000F) return ADD2_ARM;  //ADD Rd,PC

    if ((ins & 0xFF7F000) == 0xE71F000) return LDR1_ARM;  //LDR Rd,[PC,Rm]
    if ((ins & 0xFFBF0000) == 0xe59f0000) return LDR2_ARM;  //LDR Rd,[PC,#imme]
    if ((ins & 0xFFFF000F) == 0xE1A000F) return MOV_ARM;  //MOV Rd,PC

    return UNDEFINE;


}

int fixOneInsArm32(uint32_t pc, uint32_t lr, uint32_t instruction, uint32_t *trampolineIns) {
    int insType;
    int trampolinePos;
    int value;

    insType = getInsTypeArm32(instruction);
    trampolinePos = 0;
    if (insType == BX_ARM || insType == BLX1_ARM || insType == BL_ARM || insType == B_ARM) {
        int32_t imme32;
        uint32_t x;
        int topBit;
        //改成通过LDR PC实现函数跳转，手动填写LR为备份ins的下一个地址
        if (insType == BL_ARM || insType == BLX1_ARM) {
            trampolineIns[trampolinePos++] = 0xE28FE004;    //ADD LR, PC, #4
        }
        trampolineIns[trampolinePos++] = 0xE51FF004;    // LDR PC, [PC, #-4]
        //根据不同的指令，计算imme32的值
        if (insType == B_ARM || insType == BL_ARM || insType == BLX1_ARM) {
            x = (instruction & 0xFFFFFF); //取指令中的24bit立即数
            topBit = x >> 23;
            imme32 = topBit ? ((x << 2) | 0xFC000000) : x;  //若是负数，从24位符号整数转成32位整数
            //对于BLX，要设置bit[1] = H，切换成thumb模式，所以bit[0] = 1
            imme32 = (insType == BLX1_ARM) ?
                     (imme32 | (instruction & 0x1000000) >> 23) + 1
                                           : imme32;
            value = imme32 + pc + 8;
        } else {
            value = pc;
        }
        trampolineIns[trampolinePos] = (uint32_t) value;
        //fix BX BLX B BL end
    } else if (insType == ADD1_ARM || insType == ADD2_ARM) {
        int rd;
        int rm;
        int r;
        rd = (instruction & 0xF000) >> 12;
        rm = (instruction & 0xF);
        //得到不参与运算的寄存器，当做临时寄存器
        for (r = 11;; r--) {
            if (r != rd && r != rm)
                break;
        }
        trampolineIns[trampolinePos++] = 0xE52D0004 | (r << 12);    //PUSH {r};
        trampolineIns[trampolinePos++] = 0xE59F0008 | (r << 12);    //LDR r,[PC,#8]
        trampolineIns[trampolinePos++] = (insType == ADD1_ARM) ?     //将PC替换为寄存器r
                                         (instruction & 0xFFF0FFFF) | (r << 16) : //若为ADD Rd,PC,Rm形式
                                         (instruction & 0xF) | r;                //若为ADD Rd,PC形式
        trampolineIns[trampolinePos++] = 0xE49D0004 | (r << 12);    //POP {r}
        trampolineIns[trampolinePos++] = 0xE28FF000;    // ADD PC,PC,#0，清空CPU三级流水线，跳过下一条命令
        trampolineIns[trampolinePos++] = pc;
        //fix ADD end
    } else if (insType == MOV_ARM || insType == LDR1_ARM || insType == LDR2_ARM ||
               insType == ADR1_ARM ||
               insType == ADR2_ARM) {
        int rd;
        int rm;
        int r;
        int value;
        uint32_t imme32 = 0;
        rd = (instruction & 0xF000) >> 12;
        rm = (insType == LDR1_ARM) ? (instruction & 0xF) : 16;
        for (r = 12;; r--) {
            if (r != rd && r != rm)
                break;
        }
        //LDR Rd,[PC,Rm]
        if (insType == LDR1_ARM) {
            trampolineIns[trampolinePos++] = 0xE52D0004 | (r << 12);    //PUSH {r};
            trampolineIns[trampolinePos++] = 0xE59F0008 | (r << 12);    //LDR r,[PC,#8]
            trampolineIns[trampolinePos++] = (instruction & 0xFFF0FFFF) | (r << 16);//改成 LDR Rd,[r,Rm]
            trampolineIns[trampolinePos++] = 0xE49D0004 | (r << 12);    //POP {r}
            trampolineIns[trampolinePos++] = 0xE28FF000;    // ADD PC,PC,#0，清空CPU三级流水线，跳过下一条命令
            trampolineIns[trampolinePos++] = pc;
        }//TODO:have bugs(U flag)
            //LDR Rd,[PC,#imme]
        else if (insType == LDR2_ARM) {
            imme32 = instruction & 0xFFF;
            trampolineIns[trampolinePos++] = 0xE52D0004 | (r << 12);    //PUSH {r};
            trampolineIns[trampolinePos++] = 0xE59F0008 | (r << 12);    //LDR r,[PC,#8]
            trampolineIns[trampolinePos++] =
                    (instruction & 0xFFF0FFFF) | (r << 16);//改成 LDR Rd,[r,#imme]
            trampolineIns[trampolinePos++] = 0xE49D0004 | (r << 12);    //POP {r}
            trampolineIns[trampolinePos++] = 0xE28FF000;    // ADD PC,PC,#0，清空CPU三级流水线，跳过下一条命令
            trampolineIns[trampolinePos++] = pc;
        }
        else if (insType == ADR1_ARM || insType == ADR2_ARM) {
            imme32 = instruction & 0xFFF;
            value = (insType == ADR1_ARM) ? (pc + imme32 + 8) : (pc - imme32 + 8);
            trampolineIns[trampolinePos++] = 0xE51F0000 | (r << 12);    // LDR Rr, [PC]
            trampolineIns[trampolinePos++] = 0xE28FF000;    // ADD PC,PC,#0
            trampolineIns[trampolinePos++] = value;
        } else {//MOV Rd,PC
            trampolineIns[trampolinePos++] = 0xE51F0000 | (r << 12);    // LDR r, [PC]
            trampolineIns[trampolinePos++] = (instruction & 0xFFFFFFF) | r;    // MOV Rd,r
            trampolineIns[trampolinePos++] = pc;
        }
    }//fix LDR ADR MOV end
    else{//ins that don not  have to fix yet
        trampolineIns[trampolinePos++] = instruction;
    }
    return 4*trampolinePos;//返回修复后指令的总长度
}

//对外修复ARM32指令的接口
int fixInsArm32(void *fixedIns,HOOKINFO *hookinfo){
    uint32_t *pCurIns = NULL;
    int fixedOneInsLen =0;
    int fixedInsLen = 0;    //总共修复的指令长度
    int origInsLen = 0;        //保存修复源指令长度
    int pos = 0;
    uint32_t pc = 0;
    uint32_t lr = 0;    //保存跳回来的地址

    pCurIns = (uint32_t *) hookinfo->backupOpcode;
    pc = hookinfo->pHookAddr +8;
    lr = hookinfo->pHookAddr +8;
    uint32_t fixedOneInsBuf[0x40];  //存放一条源指令修复后的指令

    while(1){
        fixedOneInsLen = fixOneInsArm32(pc,lr,*pCurIns,fixedOneInsBuf);
        memcpy(fixedIns+pos, fixedOneInsBuf, fixedOneInsLen);
        pos +=fixedOneInsLen;
        pc += sizeof(uint32_t);
        fixedInsLen += fixedOneInsLen;
        origInsLen += 4;
        if(origInsLen < 8){
            pCurIns++;
        }else{  //源指令修复完成
            return fixedInsLen;
        }
    }
    LOGI("error in fixInsArm32, go to check");
    return 0;

}