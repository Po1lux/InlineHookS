//
// Created by seanchen on 2020-02-16.
//

#ifndef INLINEHOOKS_INTERFACE_H
#define INLINEHOOKS_INTERFACE_H

bool doInlineHook(void *pHookAddr, void (*onCallBack)(struct pt_regs *));

uint32_t GLRAddr;

void afterHook(struct pt_regs *regs);

#endif //INLINEHOOKS_INTERFACE_H
