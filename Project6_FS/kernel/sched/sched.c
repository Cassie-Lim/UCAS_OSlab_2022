#include <os/list.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/mm.h>
#include <os/loader.h>
#include <os/string.h>
#include <os/task.h>
#include <os/net.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>
#include <screen.h>
#include <csr.h>
#include <pgtable.h>
#include <e1000.h>
extern void ret_from_exception();


pcb_t pcb[NUM_MAX_TASK];
const ptr_t m_pid0_stack = INIT_KERNEL_STACK_M + PAGE_SIZE;
const ptr_t s_pid0_stack = INIT_KERNEL_STACK_S + PAGE_SIZE;
// uintptr_t kva_pgdir = pa2kva((uintptr_t)PGDIR_PA);
pcb_t m_pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)m_pid0_stack,
    .user_sp = (ptr_t)m_pid0_stack,
    .run_cpu_id = 1,
    .cpu_mask = 0x03,
    .pgdir = 0xffffffc051000000,
};
pcb_t s_pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)s_pid0_stack,
    .user_sp = (ptr_t)s_pid0_stack,
    .run_cpu_id = 2,
    .cpu_mask = 0x03,
    .pgdir = 0xffffffc051000000,
};

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* current running task PCB */
pcb_t * volatile current_running[NR_CPUS];

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    // TODO: [p2-task3] Check sleep queue to wake up PCBs
    check_sleeping();

    // TODO: [p2-task1] Modify the current_running[cpu_id] pointer.
    pcb_t * prior_running = current_running[cpu_id];
    task_status_t prior_stat = prior_running->status;
    prior_running->run_cpu_id = 0;
    
    if(current_running[cpu_id]->pid != 0){
        // add to the ready queue
        if(prior_stat == TASK_RUNNING){
            prior_running->status = TASK_READY;
            add_node_to_q(&prior_running->list, &ready_queue);
        }    
        // 判断前一个进程是否已经退出，是则回收
        else if(prior_stat==TASK_EXITED)
            pcb_release(prior_running);
        // 还有可能处于blocked状态
    }
    else
        prior_running->status = TASK_READY;
    current_running[cpu_id] = seek_ready_pcb();
    current_running[cpu_id]->status = TASK_RUNNING;
    current_running[cpu_id]->run_cpu_id = cpu_id + 1;
    screen_move_cursor(current_running[cpu_id]->cursor_x, current_running[cpu_id]->cursor_y);
    set_satp(SATP_MODE_SV39, current_running[cpu_id]->pid, kva2pa(current_running[cpu_id]->pgdir) >> NORMAL_PAGE_SHIFT);
    local_flush_tlb_all();
    // TODO: [p5-task3] Check send/recv queue to unblock PCBs
    // local_flush_dcache();
    // int tdt = e1000_read_reg(e1000, E1000_TDT);
    // int rdt = (e1000_read_reg(e1000, E1000_RDT)+1)%RXDESCS;
    // list_node_t *tmp;
    // // 队列非空且DD位拉高
    // if((tmp = send_block_queue.next) != &send_block_queue && (tx_desc_array[tdt].status & E1000_TXD_STAT_DD)){
    //     do_unblock(tmp);
    // }
    // if((tmp = recv_block_queue.next) != &recv_block_queue && (rx_desc_array[rdt].status & E1000_RXD_STAT_DD)){
    //     do_unblock(tmp);
    // }
    // TODO: [p2-task1] switch_to current_running[cpu_id]
    switch_to(prior_running, current_running[cpu_id]);
    return;
}

void do_sleep(uint32_t sleep_time)
{
    // TODO: [p2-task3] sleep(seconds)
    // NOTE: you can assume: 1 second = 1 `timebase` ticks
    // 1. block the current_running[cpu_id]
    current_running[cpu_id]->status = TASK_BLOCKED;
    add_node_to_q(&current_running[cpu_id]->list, &sleep_queue);
    // 2. set the wake up time for the blocked task
    current_running[cpu_id]->wakeup_time = get_timer()+sleep_time;
    // 3. reschedule because the current_running[cpu_id] is blocked.
    do_scheduler();
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: [p2-task2] block the pcb task into the block queue
    pcb_t * tmp = get_pcb_from_node(pcb_node);
    tmp->status = TASK_BLOCKED;
    add_node_to_q(pcb_node, queue);
    do_scheduler();
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: [p2-task2] unblock the `pcb` from the block queue
    delete_node_from_q(pcb_node);
    pcb_t * tmp = get_pcb_from_node(pcb_node);
    tmp->status = TASK_READY;
    add_node_to_q(pcb_node, &ready_queue);
}

// list_node_t* seek_ready_node(){
//     list_node_t *p = ready_queue.next;
//     if(p!=&ready_queue){
//         // 就绪队列非空
//         delete_node_from_q(p);
//     }
//     return p;
// }
pcb_t * seek_ready_pcb(){
    list_node_t *p = ready_queue.next;
    pcb_t * tmp;
    while(p!=&ready_queue){
        // 就绪队列非空
        tmp = get_pcb_from_node(p);
        if(tmp->cpu_mask & (cpu_id+1)){
            delete_node_from_q(p);
            return tmp;
        }
        p = p->next;
    }
    return cpu_id? &s_pid0_pcb: &m_pid0_pcb;    // fail to find the task, return to pid 0
}

void pcb_release(pcb_t* p){
    // 栈指针复位不能在此处进行，因为后续上下文切换还需要使用栈
    // // 将栈指针复位
    // int index = (p-pcb)/sizeof(pcb_t);
    // pcb[index].kernel_sp = ROUND(pcb[index].kernel_sp, PAGE_SIZE);    
    // pcb[index].user_sp = ROUND(pcb[index].user_sp, PAGE_SIZE);
    p->status = PCB_UNUSED;
    p->run_cpu_id = 0;
    p->cpu_mask = 0x03;
    // 将之从原队列删除
    delete_node_from_q(&(p->list));
    // 释放等待队列的所有进程
    free_block_list(&(p->wait_list));
    // 释放持有的所有锁
    release_all_lock(p->pid);
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
    if(p==NULL||q==NULL) return;
    p->next = q;
    q->prev = p;
    node->next = node->prev = NULL; // delete the node completely
}

pcb_t * get_pcb_from_node(list_node_t* node){
    for(int i=0;i<NUM_MAX_TASK;i++){
        if(node == &pcb[i].list)
            return &pcb[i];
    }
    return cpu_id? &s_pid0_pcb: &m_pid0_pcb;    // fail to find the task, return to pid 0
    // return 0;    
}

void free_block_list(list_node_t* head){    //释放被阻塞的进程
    list_node_t* p, *next;
    for(p=head->next; p!= head; p= next){
        next = p->next;
        do_unblock(p);
    }
}
int search_free_pcb(){  // 查找可用pcb并返回下标，若无则返回-1
    for(int i=0; i<NUM_MAX_TASK; i++){
        if(pcb[i].status==PCB_UNUSED)
            return i;
    }
    return -1;
}
// 根据pid释放所有持有的锁
void release_all_lock(pid_t pid){
    for(int i=0; i<LOCK_NUM; i++){
        if(mlocks[i].pid == pid && mlocks[i].usage == USING)
            do_mutex_lock_release(i);
    }
}

// 只有一个arg
void do_pthread_create(pid_t *thread, void (*start_routine)(void*), void *arg){
    int index = search_free_pcb();
    if(index==-1)   // 进程数已满，返回
        return 0;
    // 待修改回收机制！
    pcb[index].pgdir = current_running[cpu_id]->pgdir;
    // 栈复位
    pcb[index].kernel_sp = ROUND(pcb[index].kernel_sp, PAGE_SIZE);    
    pcb[index].user_sp = ROUND(pcb[index].user_sp, PAGE_SIZE);
    pcb[index].user_sp_kva = ROUND(pcb[index].user_sp_kva, PAGE_SIZE);
    // 建立用户栈用户虚地址和物理地址的映射
    map_page_helper(pcb[index].user_sp-PAGE_SIZE, kva2pa(pcb[index].user_sp_kva-PAGE_SIZE), pcb[index].pgdir);
    pcb[index].pid = tasknum + 1; // pid 0 is for kernel
    pcb[index].status = TASK_READY;
    pcb[index].cursor_x = current_running[cpu_id]->cursor_x;
    pcb[index].cursor_y = current_running[cpu_id]->cursor_y;
    pcb[index].cpu_mask = current_running[cpu_id]->cpu_mask;
    //初始化栈，改变入口地址，存储参数
    init_pcb_stack(pcb[index].kernel_sp, pcb[index].user_sp, start_routine, &pcb[index], arg, NULL);
    // 加入ready队列
    add_node_to_q(&pcb[index].list, &ready_queue);
    // 进程数加一
    tasknum++;
    *thread = pcb[index].pid; // 返回的pid存于指针所指示位置
}

pid_t do_exec(char *name, int argc, char *argv[]){  //创建进程，不成功返回0
    char **argv_ptr_kva;
    char ** argv_ptr;
    int index = search_free_pcb();
    // 清空页表
    pcb[index].pgdir = pcb[index].pgdir_orig;
    clear_pgdir(pcb[index].pgdir);
    // 共享kernel页表
    share_pgtable(pcb[index].pgdir, pa2kva(PGDIR_PA));
    if(index==-1)   // 进程数已满，返回
        return 0;
    uint64_t entry_point = map_task(name, pcb[index].pgdir);
    if(entry_point==0)   // 找不到相应task，返回
        return 0;
    // 创建PCB
    else{
        // 栈复位
        pcb[index].kernel_sp = ROUND(pcb[index].kernel_sp, PAGE_SIZE);    
        pcb[index].user_sp = ROUND(pcb[index].user_sp, PAGE_SIZE);
        pcb[index].user_sp_kva = ROUND(pcb[index].user_sp_kva, PAGE_SIZE);
        // 建立用户栈用户虚地址和物理地址的映射
        map_page_helper(pcb[index].user_sp-PAGE_SIZE, kva2pa(pcb[index].user_sp_kva-PAGE_SIZE), pcb[index].pgdir);
        pcb[index].pid = tasknum + 1; // pid 0 is for kernel
        pcb[index].status = TASK_READY;
        pcb[index].cursor_x = 0;
        pcb[index].cursor_y = 0;
        pcb[index].cpu_mask = current_running[cpu_id]->cpu_mask;    // 继承父进程的mask
        // 参数搬到用户栈
        pcb[index].user_sp_kva -= sizeof(char*) * argc;
        pcb[index].user_sp -= sizeof(char*) * argc;
        argv_ptr = (char**)pcb[index].user_sp;
        argv_ptr_kva = (char **)pcb[index].user_sp_kva;
        
        for(int i=argc-1; i>=0; i--){
            int len = strlen(argv[i])+1;    //要拷贝'\0'
            pcb[index].user_sp_kva -=len;
            pcb[index].user_sp -=len;
            argv_ptr_kva[i] = (char*)pcb[index].user_sp;
            // argv_ptr_kva[i] = (char*)pcb[index].user_sp_kva;
            strcpy((char*)pcb[index].user_sp_kva, argv[i]);
        }
        pcb[index].user_sp = (reg_t)ROUNDDOWN(pcb[index].user_sp, 128);    // 栈指针128字节对齐
        //初始化栈，改变入口地址，存储参数
        init_pcb_stack(pcb[index].kernel_sp, pcb[index].user_sp, entry_point, &pcb[index], argc, argv_ptr);
        // 加入ready队列
        add_node_to_q(&pcb[index].list, &ready_queue);
        // 进程数加一
        tasknum++;
    }
    return pcb[index].pid;  //返回pid值
}

void do_exit(void){
    current_running[cpu_id]->status = TASK_EXITED;
    // 清理分配给用户页的info node
    pgdir_id = get_pgdir_id(current_running[cpu_id]->pgdir);
    // 其非子线程，做用户页面的回收
    if(current_running[cpu_id]-pcb == pgdir_id){
        for(int i=0; i<USER_PAGE_MAX_NUM; i++){
            if(alloc_info[i].uva!=0 && alloc_info[i].pgdir_id==pgdir_id){
                delete_node_from_q(&alloc_info[i].lnode);
                // if(alloc_info[i].pa!=0)
                // // 放到swap_out_list之中，下次可被换出，[TODO]同时swapPage时检查对应结点uva是否全为0，若为0，回收
                // // 注意此时暂时不能清空pa域（其实也可以维护维护一个status标识物理地址是否回收，但是好麻烦……）
                //     add_node_to_q(&alloc_info[i].lnode, &swap_out_list);
                // else
                //  add_node_to_q(&alloc_info[i].lnode, &free_list);
                add_node_to_q(&alloc_info[i].lnode, &free_list);
                alloc_info[i].uva = 0;
                alloc_info[i].on_disk_sec = 0;
                alloc_info[i].pa = 0;
                alloc_info[i].pgdir_id = 0;
                page_cnt--;
            }
        }
    }


    do_scheduler();
}

int do_kill(pid_t pid){
    for(int i=0; i<NUM_MAX_TASK; i++){
        if(pcb[i].status!=PCB_UNUSED && pcb[i].pid==pid){
            // 修改进程状态
            if(pcb[i].status==TASK_RUNNING) // 在另一个核上运行
                pcb[i].status = TASK_EXITED;
            else{
                pcb[i].status = PCB_UNUSED;
                pcb_release(&pcb[i]);
            }
            // 返回1，表示找到对应进程且将其kill
            return 1;
        }
    }
    return 0;
}

int do_waitpid(pid_t pid){
    for(int i=0; i<NUM_MAX_TASK; i++){
        if(pcb[i].pid == pid){
            if(pcb[i].status != PCB_UNUSED){
            // if(pcb[i].status != TASK_EXITED && pcb[i].status != PCB_UNUSED){
                do_block(&(current_running[cpu_id]->list), &(pcb[i].wait_list));
                return pid;
            }
        }
    }
    return 0;
}

void do_process_show(){
    int i;
    static char *stat_str[4]={
        "BLOCKED","RUNNING","READY","EXITED"
    };
    screen_write("[Process table]:\n");
    for(i=0; i<NUM_MAX_TASK; i++){
        if(pcb[i].status==PCB_UNUSED)
            continue;
        else if(pcb[i].status==TASK_RUNNING)
            printk("[%d] PID : %d  STATUS : %s mask: 0x%x Running on core %d\n", i, pcb[i].pid, stat_str[pcb[i].status], pcb[i].cpu_mask, pcb[i].run_cpu_id);
        else
            printk("[%d] PID : %d  STATUS : %s mask: 0x%x\n", i, pcb[i].pid, stat_str[pcb[i].status], pcb[i].cpu_mask);
    }
}

pid_t do_getpid(){
    return current_running[cpu_id]->pid;
}

pid_t do_taskset(int mode_p, int mask, void* pid_name){
    int pid = (int)pid_name;
    if(mode_p){
        for(int i=0; i<NUM_MAX_TASK; i++){
            if(pcb[i].status!=PCB_UNUSED && pcb[i].pid == pid){
                pcb[i].cpu_mask = mask;
                return pid;
            }
        }
        printk("Fail to find task with pid %d", pid);
        return 0;
    }
    else{
        pid = do_exec((char*)pid_name, 1, NULL);
        for(int i=0; i<NUM_MAX_TASK; i++){
            if(pcb[i].status!=PCB_UNUSED && pcb[i].pid == pid)
                pcb[i].cpu_mask = mask;
        }
        return pid;
    }
}
