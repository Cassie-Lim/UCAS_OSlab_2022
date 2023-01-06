#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <type.h>
#include <screen.h>
#define SECTOR_SIZE 512
#define NBYTES2SEC(nbytes) (((nbytes) / SECTOR_SIZE) + ((nbytes) % SECTOR_SIZE != 0))

// [p1-task4]
uint64_t load_task_img(char *taskname){
    int i;
    int entry_addr;
    int start_sec;
    int blocknums;
    for(i=0;i<TASK_MAXNUM;i++){
        if(strcmp(taskname, tasks[i].taskname)==0){
            entry_addr = TASK_MEM_BASE + TASK_SIZE * i;
            start_sec = tasks[i].start_addr / SECTOR_SIZE;                      // 起始扇区：向下取整
            blocknums = NBYTES2SEC(tasks[i].task_size + tasks[i].start_addr) - tasks[i].start_addr / SECTOR_SIZE;
            bios_sdread(entry_addr, blocknums, start_sec); 
            memcpy(entry_addr, entry_addr + (tasks[i].start_addr - start_sec*512), tasks[i].task_size); 
            return entry_addr;  // 返回程序存储的起始位置
        }
    }
    // 匹配失败，提醒重新输入
    char *output_str = "Fail to find the task! Please try again!\n";
    screen_write(output_str);
    return 0;
}

uint64_t find_task(char *taskname){
    int i;
    int entry_addr;
    for(i=0;i<TASK_MAXNUM;i++){
        if(strcmp(taskname, tasks[i].taskname)==0){
            entry_addr = TASK_MEM_BASE + TASK_SIZE * i;
            return entry_addr;  // 返回程序存储的起始位置
        }
    }
    // 匹配失败，提醒重新输入
    char *output_str = "Fail to find the task! Please try again!\n";
    screen_write(output_str);
    return 0;
}
