#include <os/list.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/mm.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>

#include <os/task.h>
#include <csr.h>
extern void ret_from_exception();


pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack,
    .user_sp = (ptr_t)pid0_stack
};

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    // TODO: [p2-task3] Check sleep queue to wake up PCBs
    check_sleeping();

    // TODO: [p2-task1] Modify the current_running pointer.
    pcb_t * prior_running;
    prior_running = current_running;
    
    if(current_running->pid != 0){
        // add to the ready queue
        if(current_running->status == TASK_RUNNING){
            current_running->status = TASK_READY;
            add_node_to_q(&current_running->list, &ready_queue);
        }    
        else if(current_running->status == TASK_BLOCKED)
            add_node_to_q(&current_running->list, &sleep_queue);
    }
    list_node_t* tmp = seek_ready_node();

    current_running = get_pcb_from_node(tmp);
    current_running->status = TASK_RUNNING;
    // TODO: [p2-task1] switch_to current_running
    switch_to(prior_running, current_running);
    return;
}

void do_sleep(uint32_t sleep_time)
{
    // TODO: [p2-task3] sleep(seconds)
    // NOTE: you can assume: 1 second = 1 `timebase` ticks
    // 1. block the current_running
    current_running->status = TASK_BLOCKED;
    // 2. set the wake up time for the blocked task
    current_running->wakeup_time = get_timer()+sleep_time;
    // 3. reschedule because the current_running is blocked.
    do_scheduler();
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: [p2-task2] block the pcb task into the block queue
    pcb_t * tmp = get_pcb_from_node(pcb_node);
    tmp->status = TASK_BLOCKED;
    add_node_to_q(pcb_node, queue);
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: [p2-task2] unblock the `pcb` from the block queue
    delete_node_from_q(pcb_node);
    pcb_t * tmp = get_pcb_from_node(pcb_node);
    tmp->status = TASK_READY;
    add_node_to_q(pcb_node, &ready_queue);
}

list_node_t* seek_ready_node(){
    list_node_t *p = ready_queue.next;
    // delete p from queue
    delete_node_from_q(p);
    return p;
}

void add_node_to_q(list_node_t* node,list_head *head){
    list_node_t *p = head->prev; // tail ptr
    p->next = node;
    node->prev = p;
    node->next = head;
    head->prev = node;           // update tail ptr    
}

void delete_node_from_q(list_node_t* node){
    list_node_t* p, *q;
    p = node->prev;
    q = node->next;
    p->next = q;
    q->prev = p;
    node->next = node->prev = NULL; // delete the node completely
}

pcb_t * get_pcb_from_node(list_node_t* node){
    for(int i=0;i<NUM_MAX_TASK;i++){
        if(node == &pcb[i].list)
            return &pcb[i];
    }
    return &pid0_pcb;    // fail to find the task, return to kernel
}

int thread_create(uint64_t entry_point, int*arg){
    // assert(tasknum<NUM_MAX_TASK);
    if(tasknum>=NUM_MAX_TASK)
        return -1;
    pcb[tasknum].status = current_running->status;
    pcb[tasknum].cursor_x = current_running->cursor_x;
    pcb[tasknum].cursor_y = current_running->cursor_y;
    pcb[tasknum].kernel_sp = (reg_t) (allocKernelPage(1)+PAGE_SIZE);    //分配一页
    pcb[tasknum].user_sp = (reg_t) (allocUserPage(1)+PAGE_SIZE);
    pcb[tasknum].pid = tasknum+1;
    //初始化栈，改变入口地址，存储参数
    regs_context_t *pt_regs =
        (regs_context_t *)(pcb[tasknum].kernel_sp - sizeof(regs_context_t));
    for(int i=0; i<32; i++)
        pt_regs->regs[i]=0;
    pt_regs->regs[1] = (uint64_t) entry_point;      // ra
    pt_regs->regs[2] = pcb[tasknum].user_sp;        // sp
    pt_regs->regs[4] = (uint64_t)&pcb[tasknum];     // tp
    pt_regs->regs[10] = arg[0];                     // a0
    pt_regs->regs[11] = arg[1];                     // a1
    pt_regs->regs[12] = arg[2];                     // a2
    pt_regs->regs[13] = arg[3];                     // a3
    pt_regs->sstatus = SR_SPIE;                     // SPP set to 0, SPIE set to 1
    pt_regs->sepc = (uint64_t)entry_point;
    
    pcb[tasknum].kernel_sp -=(sizeof(switchto_context_t) + sizeof(regs_context_t));

    switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));
    pt_switchto->regs[0] = (uint64_t)ret_from_exception;     // ra   
    pt_switchto->regs[1] = pcb[tasknum].kernel_sp;                   // sp

    // add to ready queue
    add_node_to_q(&pcb[tasknum].list, &ready_queue);
    tasknum++;
    return 0;
}