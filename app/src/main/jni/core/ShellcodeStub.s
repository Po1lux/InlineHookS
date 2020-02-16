.global _shellcode_begin_s
.global _shellcode_end_s
.global _new_function_addr_s
.global _old_function_addr_s

.data

_shellcode_begin_s:
    push    {r0, r1, r2, r3}
    mrs     r0, cpsr        ;保存程序状态寄存器
    str     r0, [sp, #0xC]
    str     r14, [sp, #8]
    add     r14, sp, #0x10
    str     r14, [sp, #4]
    pop     {r0}
    push    {r0-r12}
    mov     r0, sp
    ldr     r3, _new_function_addr_s
    blx     r3
    ldr     r0, [sp, #0x3C]     ;取保存程序状态寄存器得值
    msr     cpsr, r0            ;写入程序状态寄存器
    ldmfd   sp!, {r0-r12}       ;恢复r0-r12
    ldr     r14, [sp, #4]       ;恢复lr(r14)
    ldr     sp, [r13]           ;恢复sp(r13)
    ldr     pc, _old_function_addr_s

_new_function_addr_s:
.word 0xffffffff

_old_function_addr_s:
.word 0xffffffff

_shellcode_end_s:

.end