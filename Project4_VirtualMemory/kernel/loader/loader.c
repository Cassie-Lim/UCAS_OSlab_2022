#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <os/loader.h>
#include <type.h>
#include <screen.h>
#include <os/mm.h>
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

uint64_t map_task(char *taskname, uintptr_t pgdir){
    int i;
    uint64_t entry_addr;
    uint64_t user_va, user_va_end, cpy_size;
    for(i=0;i<TASK_MAXNUM;i++){
        if(strcmp(taskname, tasks[i].taskname)==0){
            entry_addr = pa2kva(tasks[i].start_addr-tasks[0].start_addr+TASK_MEM_BASE);
            user_va_end = USER_ENTRYPOINT + tasks[i].p_memsz;
            for(user_va = USER_ENTRYPOINT; user_va< user_va_end; user_va += PAGE_SIZE, entry_addr+=PAGE_SIZE){
                uintptr_t va = alloc_page_helper(user_va, pgdir);
                cpy_size = (user_va_end-user_va)>PAGE_SIZE ? PAGE_SIZE: (user_va_end-user_va);
                // 将任务拷贝到分配的虚地址
                memcpy(va, entry_addr, cpy_size);
            }
            // // 将虚地址和实地址做映射，存于用户的页表中
            // map_page(USER_ENTRYPOINT, entry_addr, pgdir);
            // // 清空bss段
            // bzero(va+task[i].task_size, task[i].p_memsz - task[i].task_size);
            return USER_ENTRYPOINT;  // 返回用户虚地址
        }
    }
    // 匹配失败，提醒重新输入
    char *output_str = "Fail to find the task! Please try again!\n";
    screen_write(output_str);
    return 0;
}
