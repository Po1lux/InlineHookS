//
// Created by seanchen on 2020-02-16.
//

#ifndef INLINEHOOKS_INTERFACE_H
#define INLINEHOOKS_INTERFACE_H


void afterHooks(struct pt_regs *regs);

//注册hook，生成hook跳转代码
bool registerHook(char *packageName,
                  char *soPath,
                  char *funcName,
                  void(*beforeHook)(struct pt_regs *, uint32_t LRValue),
                  void(*afterHook)(struct pt_regs *));

void registerAfterHook(void *retAddr, void(*afterHook)(struct pt_regs *regs));


#endif //INLINEHOOKS_INTERFACE_H
