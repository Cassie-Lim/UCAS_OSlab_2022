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

#define NBYTES2SEC(nbytes) (((nbytes) / SECTOR_SIZE) + ((nbytes) % SECTOR_SIZE != 0))
#define SECTOR_SIZE 512

extern void ret_from_exception();

// Task info array
task_info_t tasks[TASK_MAXNUM];
char buffer [512];

int tasknum = 0; // total tasks

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
}

static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb)
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
    if(pcb->pid==0)
        pt_regs->sstatus = SR_SPP;      // kernel should not be set to user mode
    else
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
    pid0_pcb.status = TASK_RUNNING;
    pid0_pcb.list.prev = NULL;
    pid0_pcb.list.next = NULL;
    init_pcb_stack(pid0_pcb.kernel_sp, pid0_pcb.user_sp, (uint64_t)ret_from_exception, &pid0_pcb);
    // load task by name
    char taskname[10] = "";
    char *str_tmp = "a";
    int start_sec = seq_start_loc / SECTOR_SIZE;
    int blocknums = NBYTES2SEC(seq_end_loc) - start_sec;
    uint64_t entry_addr;
    bios_sdread((uint64_t)buffer, blocknums, start_sec);
    for(int i= seq_start_loc - start_sec*SECTOR_SIZE; i< seq_end_loc - start_sec*SECTOR_SIZE; i++){
        if(buffer[i]=='#'){
            entry_addr = load_task_img(taskname);
            // create a PCB
            if(entry_addr!=0){
                pcb[tasknum].kernel_sp = (reg_t)(allocKernelPage(1)+PAGE_SIZE);    //分配一页
                pcb[tasknum].user_sp = (reg_t)(allocUserPage(1)+PAGE_SIZE);
                pcb[tasknum].pid = tasknum + 1; // pid 0 is for kernel
                pcb[tasknum].status = TASK_READY;
                pcb[tasknum].cursor_x = 0;
                pcb[tasknum].cursor_y = 0;
                init_pcb_stack(pcb[tasknum].kernel_sp, pcb[tasknum].user_sp, entry_addr, &pcb[tasknum]);
                // add to ready queue
                add_node_to_q(&pcb[tasknum].list, &ready_queue);
                
                if(++tasknum > NUM_MAX_TASK)  // total tasks should be less than the threshold
                    break;
            }
            taskname[0] = '\0';
        }
        else{
            str_tmp[0]=buffer[i];
            strcat(taskname, str_tmp);
        }
    }

    /* TODO: [p2-task1] remember to initialize 'current_running' */
    current_running = &pid0_pcb;
}

static void init_syscall(void)
{
    // TODO: [p2-task3] initialize system call table.
    syscall[SYSCALL_SLEEP]          = (long (*)())do_sleep;
    syscall[SYSCALL_YIELD]          = (long (*)())do_scheduler;
    syscall[SYSCALL_WRITE]          = (long (*)())screen_write;
    syscall[SYSCALL_CURSOR]         = (long (*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH]        = (long (*)())screen_reflush;
    syscall[SYSCALL_GET_TIMEBASE]   = (long (*)())get_time_base;
    syscall[SYSCALL_GET_TICK]       = (long (*)())get_ticks;
    syscall[SYSCALL_LOCK_INIT]      = (long (*)())do_mutex_lock_init;
    syscall[SYSCALL_LOCK_ACQ]       = (long (*)())do_mutex_lock_acquire;
    syscall[SYSCALL_LOCK_RELEASE]   = (long (*)())do_mutex_lock_release;
    syscall[SYSCALL_THREAD_CREATE]  = (long (*)())thread_create;
}

int main(int app_info_loc, int app_info_size, int seq_end_loc, int seq_start_loc)
{
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

    // Init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n");

    // Init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n");

    // Init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n");

    // TODO: [p2-task4] Setup timer interrupt and enable all interrupt globally
    // NOTE: The function of sstatus.sie is different from sie's
    bios_set_timer(get_ticks()+TIMER_INTERVAL);

    // set tp to prepare for entering exception_handler_entry
    uint64_t pid0_addr = (uint64_t)&pid0_pcb;
    asm volatile(
        "nop\n"
        "mv tp, %0\n"
        :
        :"r"(pid0_addr)
    );
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
