//
// Created by seanchen on 2020-03-07.
//
#include <zconf.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <android/log.h>
#include <stdlib.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <dirent.h>
#include "ptrace_main.h"

#define CPSR_T_MASK        ( 1u << 5 )
#define TAG "cs"
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

int ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct pt_regs *regs) {
    uint32_t i;

//    for ( i = 0; i < num_params && i < 4; i ++ )
//    {
//        regs->uregs[i] = params[i];
//    }
//
//    //
//    // push remained params onto stack
//    //
//    if ( i < num_params )
//    {
//        regs->ARM_sp -= (num_params - i) * sizeof(long) ;
//        ptrace_writedata( pid, (void *)regs->ARM_sp, (uint8_t *)&params[i], (num_params - i) * sizeof(long) );
//    }

    regs->ARM_pc = addr;
    if (regs->ARM_pc & 1) {
        /* thumb */
        regs->ARM_pc &= (~1u);
        regs->ARM_cpsr |= CPSR_T_MASK;
    } else {
        /* arm */
        regs->ARM_cpsr &= ~CPSR_T_MASK;
    }


    regs->ARM_lr = 0;
    for (int i = 0; i < sizeof(regs); i++) {
        printf("%x\n", regs->uregs[i]);
    }

    if (ptrace_setregs(pid, regs) == -1
        || ptrace_continue(pid) == -1) {
        perror("ptrace call error");
        //printf("ptrace call error:%s",strerror(errno));
        return -1;
    }


    waitpid(pid, NULL, WUNTRACED);

    return 0;
}

int find_pid_of(char *process_name) {
    int id;
    pid_t pid = -1;
    DIR *dir;
    FILE *fp;
    char filename[32];
    char cmdline[256];

    struct dirent *entry;

    if (process_name == NULL)
        return -1;

    dir = opendir("/proc");
    if (dir == NULL)
        return -1;

    while ((entry = readdir(dir)) != NULL) {
        id = atoi(entry->d_name);
        if (id != 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);

                if (strcmp(process_name, cmdline) == 0) {
                    /* process found */
                    pid = id;
                    break;
                }
            }
        }
    }

    closedir(dir);

    return pid;
}

int ptrace_attach(pid_t pid) {
    if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0) {
        perror("ptrace_attach");
        //LOGI("PTRACE_ATTACH :%s\n",strerror(errno));
        return -1;
    }

    waitpid(pid, NULL, WUNTRACED);

    //DEBUG_PRINT("attached\n");

    if (ptrace(PTRACE_SYSCALL, pid, NULL, 0) < 0) {
        perror("ptrace_syscall");
        //LOGI("PTRACE_SYSCALL :%s\n",strerror(errno));
        return -1;
    }

    waitpid(pid, NULL, WUNTRACED);

    return 0;
}

int ptrace_detach(pid_t pid) {
    if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0) {
        perror("ptrace_detach");
        return -1;
    }

    return 0;
}

int ptrace_setregs(pid_t pid, struct pt_regs *regs) {

    if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {
        perror("ptrace_setregs: Can not set register values");
        return -1;
    }

    return 0;
}

int ptrace_getregs(pid_t pid, struct pt_regs *regs) {
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {
        perror("ptrace_getregs: Can not get register values");
        return -1;
    }

    return 0;
}

int ptrace_continue(pid_t pid) {
    if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0) {
        perror("ptrace_cont");
        return -1;
    }

    return 0;
}

void *get_remote_addr(pid_t target_pid, char *module_name, void *local_addr) {
    void *local_handle, *remote_handle;
    uint32_t offset = 0x1DC8;

    //local_handle = get_module_base( -1, module_name );
    remote_handle = get_module_base(target_pid, module_name);
    //LOGI("remote hooklibc.so base:%x",(uint32_t)remote_handle);

    printf("[+] get_remote_addr: local[%x], remote[%x]\n", local_handle, remote_handle);

    //return (void *)( (uint32_t)local_addr + (uint32_t)remote_handle - (uint32_t)local_handle );
    return (void *) (offset + (uint32_t) remote_handle);
}

void *get_module_base(pid_t pid, char *module_name) {
    FILE *fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];

    if (pid < 0) {
        /* self process */
        snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
    } else {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    }

    fp = fopen(filename, "r");

    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                pch = strtok(line, "-");
                addr = strtoul(pch, NULL, 16);

                if (addr == 0x8000)
                    addr = 0;

                break;
            }
        }

        fclose(fp);
    }

    return (void *) addr;
}

/**
 *
 * @param argc
 * @param argv 包名、函数名
 * @return
 */
int main(int argc, char **argv) {
    char *pkg_name;
    char *func_name;
    pid_t target_pid;
    void *remote_fun_addr;
    struct pt_regs regs, original_regs;

    if (argc != 3) {
        printf("args error\n");
        //printf("process_name targetmethod_name libhook_path\n");
        return 0;
    } else {
        pkg_name = argv[1];
        func_name = argv[2];
        printf("pkgname is %s\n", pkg_name);
        printf("func_name is %s\n", func_name);
    }

    printf("//-----------start inject-----------//\n");
    target_pid = find_pid_of(pkg_name);
    printf("target_pid:%d", target_pid);
    remote_fun_addr = get_remote_addr(target_pid, "hooklibc.so", 0);
    if (ptrace_attach(target_pid) == -1) {
        return 0;
    }
    if (ptrace_getregs(target_pid, &regs) == -1) {
        return 0;
    }
    /* save original registers */
    memcpy(&original_regs, &regs, sizeof(regs));
    if (ptrace_call(target_pid, (uint32_t) remote_fun_addr, 0, 0, &regs) == -1) {
        return 0;
    }
    ptrace_setregs(target_pid, &regs);
    ptrace_detach(target_pid);

    printf("//-----------end inject-----------//");

    return 1;

}
