#include <os/task.h>
#include <os/string.h>
#include <os/bios.h>
#include <type.h>

// [p1-task3]
// uint64_t load_task_img(int taskid)
// {
//     /**
//      * TODO:
//      * 1. [p1-task3] load task from image via task id, and return its entrypoint
//      * 2. [p1-task4] load task via task name, thus the arg should be 'char *taskname'
//      */
//     uint64_t entry_addr;
//     char info[] = "Loading task _ ...\n\r";
//     for(int i=0;i<strlen(info);i++){
//         if(info[i]!='_') bios_putchar(info[i]);
//         else bios_putchar(taskid +'0');
//     }
//     entry_addr = TASK_MEM_BASE + TASK_SIZE * (taskid - 1);
    
//     bios_sdread(entry_addr, 15, 1 + taskid * 15);
//     return entry_addr;
// }
int have_load[TASK_MAXNUM];
// [p1-task4]
uint64_t load_task_img(char *taskname){
    int i;
    int entry_addr;
    int start_sec;
    for(i=0;i<TASK_MAXNUM;i++){
        if(strcmp(taskname, tasks[i].taskname)==0){
            entry_addr = TASK_MEM_BASE + TASK_SIZE * i;
            start_sec = tasks[i].start_addr / 512;                      // 起始扇区：向下取整
            bios_sdread(entry_addr, tasks[i].blocknums, start_sec);  
            return entry_addr + (tasks[i].start_addr - start_sec*512);  // 返回程序存储的起始位置
        }
    }
    // 匹配失败，提醒重新输入
    char *output_str = "Fail to find the task! Please try again!";
    for(i=0; i<strlen(output_str); i++){
        bios_putchar(output_str[i]);
    }
    bios_putchar('\n');
    return 0;
}

// [p1-task5]
uint64_t load_task_byid(int taskid){
    uint64_t entry_addr;
    int tmp, start_sec;
    // 判断输入id是否合法（1~16）
    if(taskid<=0 || taskid > TASK_MAXNUM)
        return 0;
    // 输出读入的程序信息
    char info[] = "Loading task _ ...\n\r";
    for(int i=0;i<strlen(info);i++){
        if(info[i]!='_') bios_putchar(info[i]);
        else{
            tmp = taskid;
            while(tmp){
                bios_putchar(tmp % 10 +'0');
                tmp /= 10;
            }
            bios_putchar(' ');
            for(int j=0; j<strlen(tasks[taskid-1].taskname); j++)
                bios_putchar(tasks[taskid-1].taskname[j]);
        }
    }
    start_sec =  tasks[taskid-1].start_addr / 512;
    entry_addr = TASK_MEM_BASE + TASK_SIZE * (taskid - 1);
    bios_sdread(entry_addr, tasks[taskid-1].blocknums, start_sec);
    return entry_addr + (tasks[taskid-1].start_addr - start_sec*512);
}
