//
// Created by seanchen on 2020-02-15.
//

#ifndef INLINEHOOKS_UTILS_H
#define INLINEHOOKS_UTILS_H
void *my_dlopen(char *libPath);
void *my_dlsym(void *ctx,char *symName);
#endif //INLINEHOOKS_UTILS_H
