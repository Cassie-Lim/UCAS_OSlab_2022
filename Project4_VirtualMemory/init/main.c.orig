#include <common.h>
#include <asm.h>
#include <asm/unistd.h>
#include <os/loader.h>
#include <os/irq.h>
#include <os/sched.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/mm.h>
#include <os/time.h>
#include <sys/syscall.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>
#include <type.h>
#include <csr.h>
#include <os/smp.h>
#define NBYTES2SEC(nbytes) (((nbytes) / SECTOR_SIZE) + ((nbytes) % SECTOR_SIZE != 0))
#define SECTOR_SIZE 512

extern void ret_from_exception();

// Task info array
task_info_t tasks[TASK_MAXNUM];
char buffer [SECTOR_SIZE*2];

int tasknum = 0; // 总的task数（含已经被kill的）

static void init_jmptab(void)
{
    volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

    jmptab[CONSOLE_PUTSTR]  = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ]         = (long (*)())sd_read;
    jmptab[QEMU_LOGGING]    = (long (*)())qemu_logging;
    jmptab[SET_TIMER]       = (long (*)())set_timer;
    jmptab[READ_FDT]        = (long (*)())read_fdt;
    jmptab[MOVE_CURSOR]     = (long (*)())screen_move_cursor;
    jmptab[PRINT]           = (long (*)())printk;
    jmptab[YIELD]           = (long (*)())do_scheduler;
    jmptab[MUTEX_INIT]      = (long (*)())do_mutex_lock_init;
    jmptab[MUTEX_ACQ]       = (long (*)())do_mutex_lock_acquire;
    jmptab[MUTEX_RELEASE]   = (long (*)())do_mutex_lock_release;
}
// 在初始化时就读入全部任务
static void load_tasks()
{
    int entry_addr;
    int start_sec;
    int blocknums;
    for(int i=0; i<TASK_MAXNUM; i++){
        if(tasks[i].taskname[0]!='\0'){
            entry_addr = TASK_MEM_BASE + TASK_SIZE * i;
            start_sec = tasks[i].start_addr / SECTOR_SIZE;                      // 起始扇区：向下取整
            blocknums = NBYTES2SEC(tasks[i].task_size + tasks[i].start_addr) - tasks[i].start_addr / SECTOR_SIZE;
            bios_sdread(entry_addr, blocknums, start_sec); 
            memcpy(entry_addr, entry_addr + (tasks[i].start_addr - start_sec*512), tasks[i].task_size); 
        }
    }
}
static void init_task_info(int app_info_loc, int app_info_size)
{
    // TODO: [p1-task4] Init 'tasks' array via reading app-info sector
    // NOTE: You need to get some related arguments from bootblock first
    int start_sec, blocknums;
    start_sec = app_info_loc / SECTOR_SIZE;
    blocknums = NBYTES2SEC(app_info_loc + app_info_size) - start_sec;
    bios_sdread((uint64_t)buffer, blocknums, start_sec);
    uint8_t *tmp = (uint8_t *)((uint64_t)buffer + app_info_loc -start_sec * SECTOR_SIZE);
    memcpy((uint8_t *)tasks, tmp, app_info_size);
    load_tasks();
}

void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb, int argc, char* argv[])
{
     /* TODO: [p2-task3] initialization of registers on kernel stack
      * HINT: sp, ra, sepc, sstatus
      * NOTE: To run the task in user mode, you should set corresponding bits
      *     of sstatus(SPP, SPIE, etc.).
      */
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    for(int i=0; i<32; i++)
        pt_regs->regs[i]=0;
    pt_regs->regs[1] = (uint64_t) entry_point;           // ra
    pt_regs->regs[2] = user_stack;                      // sp
    pt_regs->regs[4] = (uint64_t)pcb;                             // tp
    pt_regs->regs[10] = (reg_t)argc;                     // a0 = argc
    pt_regs->regs[11] = (reg_t)argv;                     // a1 = argv
    pt_regs->sstatus = SR_SPIE;  // SPP set to 0, SPIE set to 1
    pt_regs->sepc = (uint64_t)entry_point;

    /* TODO: [p2-task1] set sp to simulate just returning from switch_to
     * NOTE: you should prepare a stack, and push some values to
     * simulate a callee-saved context.
     */
    switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));
    pcb->kernel_sp = kernel_stack - sizeof(switchto_context_t) - sizeof(regs_context_t);
    pt_switchto->regs[0] = (uint64_t)ret_from_exception;     // ra   
    // pt_switchto->regs[0] = (uint64_t)entry_point;     // ra        
    pt_switchto->regs[1] = pcb->kernel_sp;  // sp

}

static void init_pcb(int seq_start_loc, int seq_end_loc)
{
    /* TODO: [p2-task1] load needed tasks and init their corresponding PCB */
    
    // PCB for kernel
    m_pid0_pcb.status =  TASK_RUNNING;
    m_pid0_pcb.list.prev = NULL;
    m_pid0_pcb.list.next = NULL;
    s_pid0_pcb.status =  TASK_READY;
    s_pid0_pcb.list.prev = NULL;
    s_pid0_pcb.list.next = NULL;
    // // 读入存储的程序序列
    // int start_sec = seq_start_loc / SECTOR_SIZE;
    // int blocknums = NBYTES2SEC(seq_end_loc) - start_sec;
    // bios_sdread((uint64_t)buffer, blocknums, start_sec);

    // 将所有pcb置为unused，并初始化wait_list\list\cpu_id域
    for(int i=0; i<NUM_MAX_TASK; i++){
        pcb[i].status = PCB_UNUSED;
        pcb[i].wait_list.prev = pcb[i].wait_list.next = &pcb[i].wait_list;
        pcb[i].list.prev = pcb[i].list.next = NULL;
        pcb[i].run_cpu_id = 0;
        pcb[i].cpu_mask = 0x03;
        // 初始化时就为每个PCB绑定栈空间，便于后续回收
        pcb[i].kernel_sp = (reg_t)(allocKernelPage(1)+PAGE_SIZE);
        pcb[i].user_sp = (reg_t)(allocUserPage(1)+PAGE_SIZE);
    }
    /* TODO: [p2-task1] remember to initialize 'current_running[cpu_id]' */
    current_running[0] = &m_pid0_pcb;
    current_running[1] = &s_pid0_pcb;
    // 创建首个用户进程shell（主从核共用？）
    bios_putchar(do_exec("shell", 0, NULL));    //创建不成功则exit
}

static void init_syscall(void)
{
    // TODO: [p2-task3] initialize system call table.
    syscall[SYSCALL_EXEC]           = (long (*)())do_exec;
    syscall[SYSCALL_EXIT]           = (long (*)())do_exit;
    syscall[SYSCALL_SLEEP]          = (long (*)())do_sleep;
    syscall[SYSCALL_KILL]           = (long (*)())do_kill;
    syscall[SYSCALL_WAITPID]        = (long (*)())do_waitpid;
    syscall[SYSCALL_PS]             = (long (*)())do_process_show;
    syscall[SYSCALL_GETPID]         = (long (*)())do_getpid;
    syscall[SYSCALL_YIELD]          = (long (*)())do_scheduler;
    syscall[SYSCALL_WRITE]          = (long (*)())screen_write;
    syscall[SYSCALL_READCH]         = (long (*)())bios_getchar;
    syscall[SYSCALL_CURSOR]         = (long (*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH]        = (long (*)())screen_reflush;
    syscall[SYSCALL_CLEAR]          = (long (*)())screen_clear;
    syscall[SYSCALL_GET_TIMEBASE]   = (long (*)())get_time_base;
    syscall[SYSCALL_GET_TICK]       = (long (*)())get_ticks;
    syscall[SYSCALL_LOCK_INIT]      = (long (*)())do_mutex_lock_init;
    syscall[SYSCALL_LOCK_ACQ]       = (long (*)())do_mutex_lock_acquire;
    syscall[SYSCALL_LOCK_RELEASE]   = (long (*)())do_mutex_lock_release;
    // syscall[SYSCALL_SHOW_TASK]      = (long (*)());
    syscall[SYSCALL_BARR_INIT]      = (long (*)())do_barrier_init;
    syscall[SYSCALL_BARR_WAIT]      = (long (*)())do_barrier_wait;
    syscall[SYSCALL_BARR_DESTROY]   = (long (*)())do_barrier_destroy;
    syscall[SYSCALL_COND_INIT]      = (long (*)())do_condition_init;
    syscall[SYSCALL_COND_WAIT]      = (long (*)())do_condition_wait;
    syscall[SYSCALL_COND_SIGNAL]    = (long (*)())do_condition_signal;
    syscall[SYSCALL_COND_BROADCAST] = (long (*)())do_condition_broadcast;
    syscall[SYSCALL_COND_DESTROY]   = (long (*)())do_condition_destroy;
    syscall[SYSCALL_MBOX_OPEN]      = (long int (*)())&do_mbox_open;
    syscall[SYSCALL_MBOX_CLOSE]     = (long int (*)())&do_mbox_close;
    syscall[SYSCALL_MBOX_SEND]      = (long int (*)())&do_mbox_send;
    syscall[SYSCALL_MBOX_RECV]      = (long int (*)())&do_mbox_recv;
    // 待补充
    syscall[SYSCALL_TASKSET]        = (long (*)())do_taskset;
    syscall[SYSCALL_PUTCHAR]        = (long (*)())screen_putchar;
    syscall[SYSCALL_THREAD_CREATE]  = (long (*)())do_thread_create;
}

int main(int app_info_loc, int app_info_size, int seq_end_loc, int seq_start_loc)
{
    cpu_id = get_current_cpu_id(); 
    // 主核的任务：全局资源初始化
    if(cpu_id==0){
        // 初始化大内核锁并上锁
        smp_init();
        lock_kernel();

        // Init jump table provided by kernel and bios(ΦωΦ)
        init_jmptab();

        // Init task information (〃'▽'〃)
        init_task_info(app_info_loc, app_info_size);

        // Init Process Control Blocks |•'-'•) ✧
        init_pcb(seq_start_loc, seq_end_loc);

        // Read CPU frequency (｡•ᴗ-)_
        time_base = bios_read_fdt(TIMEBASE);

        // Init lock mechanism o(´^｀)o
        init_locks();
        printk("> [INIT] Lock mechanism initialization succeeded.\n");

        init_barriers();
        printk("> [INIT] Barrier mechanism initialization succeeded.\n");

        init_conditions();
        printk("> [INIT] Condition mechanism initialization succeeded.\n");

        init_mbox();
        printk("> [INIT] Mailbox mechanism initialization succeeded.\n");

        // Init interrupt (^_^)
        init_exception();
        printk("> [INIT] Interrupt processing initialization succeeded.\n");

        // Init system call table (0_0)
        init_syscall();
        printk("> [INIT] System call initialized successfully.\n");

        // Init screen (QAQ)
        init_screen();
        printk("> [INIT] SCREEN initialization succeeded.\n");

        // 释放大内核锁，唤醒从核
        unlock_kernel();
        wakeup_other_hart(NULL);
        // 重新抢内核锁
        lock_kernel();
        
    }
    // 从核
    else{
        cpu_id = 1; // 强制置为1，避免出现其id不为1而下标越界的情况
        lock_kernel();
        current_running[cpu_id]->status = TASK_RUNNING;  
    }
    // set up the entrypoint of exceptions, enable interrupt globally
    setup_exception(); 
    // TODO: [p2-task4] Setup timer interrupt and enable all interrupt globally
    // NOTE: The function of sstatus.sie is different from sie's
    bios_set_timer(get_ticks()+TIMER_INTERVAL);
    printk("> [INIT] CPU %d initialization succeeded.\n", cpu_id);

    unlock_kernel();
    // Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
    while (1)
    {
        // If you do non-preemptive scheduling, it's used to surrender control
        // do_scheduler();

        // If you do preemptive scheduling, they're used to enable CSR_SIE and wfi
        enable_preempt();
        asm volatile("wfi");
    }

    return 0;
}
