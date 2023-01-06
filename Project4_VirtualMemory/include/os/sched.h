/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *        Process scheduling related content, such as: scheduler, process blocking,
 *                 process wakeup, process creation, process kill, etc.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#include <type.h>
#include <os/list.h>
#include <os/smp.h>


#define NUM_MAX_TASK 16

/* used to save register infomation */
typedef struct regs_context
{
    /* Saved main processor registers.*/
    reg_t regs[32];

    /* Saved special registers. */
    reg_t sstatus;
    reg_t sepc;
    reg_t sbadaddr;
    reg_t scause;
} regs_context_t;

/* used to save register infomation in switch_to */
typedef struct switchto_context
{
    /* Callee saved registers.*/
    reg_t regs[14];
} switchto_context_t;

typedef enum {
    TASK_BLOCKED,
    TASK_RUNNING,
    TASK_READY,
    TASK_EXITED,
    PCB_UNUSED,
} task_status_t;

/* Process Control Block */
typedef struct pcb
{
    /* register context */
    // NOTE: this order must be preserved, which is defined in regs.h!!
    reg_t kernel_sp;
    reg_t user_sp;
    ptr_t kernel_stack_base;
    ptr_t user_stack_base;

    /* previous, next pointer */
    list_node_t list;
    list_head wait_list;

    /* pgdir */
    uintptr_t pgdir;

    /* process id */
    pid_t pid;

    /* BLOCK | READY | RUNNING */
    task_status_t status;

    /* cursor position */
    int cursor_x;
    int cursor_y;

    /* time(seconds) to wake up sleeping PCB */
    uint64_t wakeup_time;

    // 运行当前进程的cpu
    uint64_t run_cpu_id;
    // 绑定cpu的掩码
    uint64_t cpu_mask;
    // 绑定的用户栈内核虚地址
    reg_t user_sp_kva;
    // 用于pgdir的复位（线程的case）
    uintptr_t pgdir_orig;
} pcb_t;

/* ready queue to run */
extern list_head ready_queue;
extern list_node_t *rq_tail;
/* sleep queue to be blocked in */
extern list_head sleep_queue;

/* current running task PCB */
extern pcb_t * volatile current_running[NR_CPUS];
extern pid_t process_id;

extern pcb_t pcb[NUM_MAX_TASK];
extern pcb_t m_pid0_pcb;
extern pcb_t s_pid0_pcb;
extern const ptr_t m_pid0_stack;
extern const ptr_t s_pid0_stack;
extern uintptr_t kva_pgdir;

extern void init_pcb_stack(ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb, int argc, char* argv[]);    // 初始化pcb栈
int search_free_pcb();  // 查找可用pcb并返回下标，若无则返回-1
extern void switch_to(pcb_t *prev, pcb_t *next);
void do_scheduler(void);
void do_sleep(uint32_t);

pcb_t * get_pcb_from_node(list_node_t* node);
pcb_t * seek_ready_pcb();
void pcb_release(pcb_t* p);

void do_block(list_node_t *, list_head *queue);
void do_unblock(list_node_t *);
void free_block_list(list_node_t* head);

/* TODO [P3-TASK1] exec exit kill waitpid ps*/
#ifdef S_CORE
extern pid_t do_exec(int id, int argc, uint64_t arg0, uint64_t arg1, uint64_t arg2);
#else
extern pid_t do_exec(char *name, int argc, char *argv[]);
#endif
extern void do_exit(void);
extern int do_kill(pid_t pid);
extern int do_waitpid(pid_t pid);
extern void do_process_show();
extern pid_t do_getpid();

extern pid_t do_taskset(int mode_p, int mask, void* pid_name);
extern void do_pthread_create(pid_t *thread, void (*start_routine)(void*), void *arg);
#endif
