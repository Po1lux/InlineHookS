//
// Created by seanchen on 2020-02-15.
//

#include <stdio.h>
#include "utils.h"
#include <include/shared.h>
#include <string.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/mman.h>
#include <elf.h>


#define __arm__
#ifdef __arm__
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym  Elf32_Sym
#elif defined(__aarch64__)
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym  Elf64_Sym
#else
#error "Arch is unknown"
#endif

typedef struct ctxInfo {
    void *execAddr;  //可执行段的地址
    void *dynstr;       //动态字串表
    void *dynsym;       //动态符号表
    int symsNum;          //符号个数
    off_t off;         //VA与文件中位置的差
} CTXINFO;

void *my_dlopen(char *libPath) {
    FILE *maps;
    char buf[0x100];
    char *libName = strrchr(libPath, '/')+1;
    uint32_t addr;
    off_t size;
    int fd;
    Elf32_Ehdr *elf = MAP_FAILED;
    CTXINFO *ctx = 0;
    void *shtoff;  //节头表地址
    void *shtAddr;
    uint32_t shtNum;    //节数量
    uint32_t shtSize;   //每个节大小

//找到so模块
    if ((maps = fopen("/proc/self/maps", "r")) == NULL) {
        LOGI("open maps error");
        return NULL;
    }
//获取so执行段地址
    while (fgets(buf, sizeof(buf), maps)) {
        if (strstr(buf, "r-xp") && strstr(buf, libName))
            break;
    }
    if (sscanf(buf, "%x", &addr) != 1) {
        LOGI("get addr error");
        return NULL;
    }
    fclose(maps);
//打开so文件
    if ((fd = open(libPath, O_RDONLY) )== -1) {
        LOGI("open so error");
        return NULL;
    }
//获得文件大小
    size = lseek(fd, 0, SEEK_END);
//在内存映射该文件，类型为elf
    if ((elf = (Elf32_Ehdr *) mmap(0, size, PROT_READ, MAP_SHARED, fd, 0) )== MAP_FAILED) {
        LOGI("mmap so failed");
        close(fd);
        return NULL;
    }
    close(fd);
    ctx = (CTXINFO *) calloc(1, sizeof(CTXINFO));
    ctx->execAddr = addr;
    shtoff = elf->e_phoff; //节头表偏移
    shtNum = elf->e_shnum;
    shtSize = elf->e_shentsize;
    shtAddr = ((void *) elf) + elf->e_shoff;

    for (uint32_t i = 0; i < shtNum; i++, shtAddr += shtSize) {
        Elf_Shdr *sh = (Elf_Shdr *)shtAddr;
        switch (sh->sh_type) {
            case SHT_DYNSYM://11
                ctx->dynsym = calloc(1, sh->sh_size);
                memcpy(ctx->dynsym, ((void *) elf + sh->sh_offset), sh->sh_size);
                ctx->symsNum = sh->sh_size / sizeof(Elf_Sym);
                ctx->off = sh->sh_addr - sh->sh_offset;
                break;
            case SHT_STRTAB://3
                ctx->dynstr = calloc(1, sh->sh_size);
                memcpy(ctx->dynstr, (void *) elf + sh->sh_offset, sh->sh_size);
                break;
            case SHT_PROGBITS:
                break;
        }
        if (ctx->dynstr && ctx->dynsym) {
            break;
        }
    }
    munmap(elf,size);
    return ctx;
}

void *my_dlsym(void *ctxStruct,char *symName){
    if(ctxStruct==NULL){
        return NULL;
    }
    CTXINFO *ctx = (CTXINFO*)ctxStruct;
    Elf_Sym *sym =(Elf_Sym*) ctx->dynsym;
    char *dynstrAddr = ctx->dynstr;
    int i;
    for(i=0;i<ctx->symsNum;i++,sym++){
        char *name = dynstrAddr+sym->st_name;
        if(strcmp(name,symName)==0){
            void *res = ctx->execAddr+sym->st_value-ctx->off;
            return res;
        }
    }
    return NULL;

}