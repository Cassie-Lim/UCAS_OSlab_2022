#ifndef __INCLUDE_TASK_H__
#define __INCLUDE_TASK_H__

#include <type.h>

#define TASK_MEM_BASE    0x56000000
// #define TASK_MEM_BASE    0x52000000
#define TASK_MAXNUM      32
#define TASK_SIZE        0x10000
#define USER_ENTRYPOINT  0x10000
/* TODO: [p1-task4] implement your own task_info_t! */
typedef struct {
    char taskname[20];
    uint32_t start_addr;
    // int blocknums;
    uint32_t task_size;
    uint32_t p_memsz;
} task_info_t;

extern task_info_t tasks[TASK_MAXNUM];
extern int tasknum;
#endif