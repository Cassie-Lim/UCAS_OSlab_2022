#include <syscall.h>
#include <stdint.h>
#include <unistd.h>

static const long IGNORE = 0L;

static long invoke_syscall(long sysno, long arg0, long arg1, long arg2,
                           long arg3, long arg4)
{
    long ret_value;
    // register long _sysno asm("a7") = sysno;
    /* TODO: [p2-task3] implement invoke_syscall via inline assembly */
    asm volatile(
        "nop\n"
        "mv a7, %1\n"
        "mv a0, %2\n"
        "mv a1, %3\n"
        "mv a2, %4\n"
        "mv a3, %5\n"
        "mv a4, %6\n"
        "ecall\n"
        "mv %0, a0\n"
        :"=r"(ret_value)
        :"r"(sysno),"r"(arg0),"r"(arg1),"r"(arg2),"r"(arg3),"r"(arg4)
    );  // store return value in a variable

    return ret_value;
}

void sys_yield(void)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_yield */
    invoke_syscall(SYSCALL_YIELD, 0, 0, 0, 0, 0);
}

void sys_move_cursor(int x, int y)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_move_cursor */
    invoke_syscall(SYSCALL_CURSOR, x, y, 0, 0, 0);
}

void sys_write(char *buff)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_write */
    invoke_syscall(SYSCALL_WRITE, (long)buff, 0, 0, 0, 0);
}

void sys_reflush(void)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_reflush */
    invoke_syscall(SYSCALL_REFLUSH, 0, 0, 0, 0, 0);
}

void sys_clear(void){
    invoke_syscall(SYSCALL_CLEAR, 0, 0, 0, 0, 0);
}

int sys_mutex_init(int key)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_init */
    return invoke_syscall(SYSCALL_LOCK_INIT, key, 0, 0, 0, 0);
}

void sys_mutex_acquire(int mutex_idx)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_acquire */
    invoke_syscall(SYSCALL_LOCK_ACQ, mutex_idx, 0, 0, 0, 0);
}

void sys_mutex_release(int mutex_idx)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_release */
    invoke_syscall(SYSCALL_LOCK_RELEASE, mutex_idx, 0, 0, 0, 0);
}

long sys_get_timebase(void)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_get_timebase */
    return invoke_syscall(SYSCALL_GET_TIMEBASE, 0, 0, 0, 0, 0);
}

long sys_get_tick(void)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_get_tick */
    return invoke_syscall(SYSCALL_GET_TICK, 0, 0, 0, 0, 0);
}

void sys_sleep(uint32_t time)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_sleep */
    invoke_syscall(SYSCALL_SLEEP, time, 0, 0, 0, 0);
}

// S-core
// pid_t  sys_exec(int id, int argc, uint64_t arg0, uint64_t arg1, uint64_t arg2)
// {
//     /* TODO: [p3-task1] call invoke_syscall to implement sys_exec for S_CORE */
// }    
// A/C-core
pid_t  sys_exec(char *name, int argc, char *argv[])
{
    /* TODO: [p3-task1] call invoke_syscall to implement sys_exec */
    return invoke_syscall(SYSCALL_EXEC, name, argc, argv, 0, 0);
}


void sys_exit(void)
{
    /* TODO: [p3-task1] call invoke_syscall to implement sys_exit */
    invoke_syscall(SYSCALL_EXIT, 0, 0, 0, 0, 0);
}

int  sys_kill(pid_t pid)
{
    /* TODO: [p3-task1] call invoke_syscall to implement sys_kill */
    return invoke_syscall(SYSCALL_KILL, pid, 0, 0, 0, 0);
}

int  sys_waitpid(pid_t pid)
{
    /* TODO: [p3-task1] call invoke_syscall to implement sys_waitpid */
    return invoke_syscall(SYSCALL_WAITPID, pid, 0, 0, 0, 0);
}


void sys_ps(void)
{
    /* TODO: [p3-task1] call invoke_syscall to implement sys_ps */
    invoke_syscall(SYSCALL_PS, 0, 0, 0, 0, 0);
}

pid_t sys_getpid()
{
    /* TODO: [p3-task1] call invoke_syscall to implement sys_getpid */
    return invoke_syscall(SYSCALL_GETPID, 0, 0, 0, 0, 0);
}

int  sys_getchar(void)
{
    /* TODO: [p3-task1] call invoke_syscall to implement sys_getchar */
    return invoke_syscall(SYSCALL_READCH, 0, 0, 0, 0, 0);
}

int  sys_barrier_init(int key, int goal)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_barrier_init */
    return invoke_syscall(SYSCALL_BARR_INIT, key, goal, 0, 0, 0);
}

void sys_barrier_wait(int bar_idx)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_barrie_wait */
    return invoke_syscall(SYSCALL_BARR_WAIT, bar_idx, 0, 0, 0, 0);
}

void sys_barrier_destroy(int bar_idx)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_barrie_destory */
    return invoke_syscall(SYSCALL_BARR_DESTROY, bar_idx, 0, 0, 0, 0);
}

int sys_condition_init(int key)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_condition_init */
    return invoke_syscall(SYSCALL_COND_INIT, key, 0, 0, 0, 0);
}

void sys_condition_wait(int cond_idx, int mutex_idx)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_condition_wait */
    invoke_syscall(SYSCALL_COND_WAIT, cond_idx, mutex_idx, 0, 0, 0);
}

void sys_condition_signal(int cond_idx)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_condition_signal */
    invoke_syscall(SYSCALL_COND_SIGNAL, cond_idx, 0, 0, 0, 0);
}

void sys_condition_broadcast(int cond_idx)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_condition_broadcast */
    invoke_syscall(SYSCALL_COND_BROADCAST, cond_idx, 0, 0, 0, 0);
}

void sys_condition_destroy(int cond_idx)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_condition_destroy */
    invoke_syscall(SYSCALL_COND_DESTROY, cond_idx, 0, 0, 0, 0);
}

int sys_mbox_open(char * name)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_mbox_open */
    return invoke_syscall(SYSCALL_MBOX_OPEN, name, 0, 0, 0, 0);
}

void sys_mbox_close(int mbox_id)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_mbox_close */
    invoke_syscall(SYSCALL_MBOX_CLOSE, mbox_id, 0, 0, 0, 0);
}

int sys_mbox_send(int mbox_idx, void *msg, int msg_length)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_mbox_send */
    return invoke_syscall(SYSCALL_MBOX_SEND, mbox_idx, msg, msg_length, 0, 0);
}

int sys_mbox_recv(int mbox_idx, void *msg, int msg_length)
{
    /* TODO: [p3-task2] call invoke_syscall to implement sys_mbox_recv */
    return invoke_syscall(SYSCALL_MBOX_RECV, mbox_idx, msg, msg_length, 0, 0);
}

void* sys_shmpageget(int key)
{
    /* TODO: [p4-task5] call invoke_syscall to implement sys_shmpageget */
    return invoke_syscall(SYSCALL_SHM_GET, key, 0, 0, 0, 0);
}

void sys_shmpagedt(void *addr)
{
    /* TODO: [p4-task5] call invoke_syscall to implement sys_shmpagedt */
    invoke_syscall(SYSCALL_SHM_DT, addr, 0, 0, 0, 0);
}

// 自己加的系统调用
uint64_t sys_copy_on_write(uint64_t uva){
    invoke_syscall(SYSCALL_COW, uva, 0, 0, 0, 0);
}
uint64_t sys_uva2pa(uint64_t uva){
    invoke_syscall(SYSCALL_UVA2PA, uva, 0, 0, 0, 0);
}
pid_t sys_taskset(int mode_p, int mask, void* pid_name){
    invoke_syscall(SYSCALL_TASKSET, mode_p, mask, pid_name, 0, 0);
}
void sys_putchar(char ch){
    invoke_syscall(SYSCALL_PUTCHAR, ch, 0, 0, 0, 0);
}
void sys_pthread_create(pthread_t *thread, void (*start_routine)(void*), void *arg){
    invoke_syscall(SYSCALL_THREAD_CREATE, thread, start_routine, arg, 0, 0);
}


int sys_net_send(void *txpacket, int length)
{
    /* TODO: [p5-task1] call invoke_syscall to implement sys_net_send */
    return invoke_syscall(SYSCALL_NET_SEND, txpacket, length, 0, 0, 0);
}

int sys_net_recv(void *rxbuffer, int pkt_num, int *pkt_lens)
{
    /* TODO: [p5-task2] call invoke_syscall to implement sys_net_recv */
    return invoke_syscall(SYSCALL_NET_RECV, rxbuffer, pkt_num, pkt_lens, 0, 0);
}

int sys_mkfs(int force_flag)
{
    // TODO [P6-task1]: Implement sys_mkfs
    return invoke_syscall(SYSCALL_FS_MKFS, force_flag, 0, 0, 0, 0);  // sys_mkfs succeeds
}

int sys_statfs(void)
{
    // TODO [P6-task1]: Implement sys_statfs
    return invoke_syscall(SYSCALL_FS_STATFS, 0, 0, 0, 0, 0);  // sys_statfs succeeds
}

int sys_cd(char *path)
{
    // TODO [P6-task1]: Implement sys_cd
    return invoke_syscall(SYSCALL_FS_CD, path, 0, 0, 0, 0);  // sys_cd succeeds
}

int sys_mkdir(char *path)
{
    // TODO [P6-task1]: Implement sys_mkdir
    return invoke_syscall(SYSCALL_FS_MKDIR, path, 0, 0, 0, 0);  // sys_mkdir succeeds
}

int sys_rmdir(char *path)
{
    // TODO [P6-task1]: Implement sys_rmdir
    return invoke_syscall(SYSCALL_FS_RMDIR, path, 0, 0, 0, 0);  // sys_rmdir succeeds
}

int sys_ls(char *path, int option)
{
    // TODO [P6-task1]: Implement sys_ls
    // Note: argument 'option' serves for 'ls -l' in A-core
    return invoke_syscall(SYSCALL_FS_LS, path, option, 0, 0, 0);  // sys_ls succeeds
}

int sys_touch(char *path)
{
    // TODO [P6-task2]: Implement sys_touch
    return invoke_syscall(SYSCALL_FS_TOUCH, path, 0, 0, 0, 0);  // sys_touch succeeds
}

int sys_cat(char *path)
{
    // TODO [P6-task2]: Implement sys_cat
    return invoke_syscall(SYSCALL_FS_CAT, path, 0, 0, 0, 0);  // sys_cat succeeds
}

int sys_fopen(char *path, int mode)
{
    // TODO [P6-task2]: Implement sys_fopen
    return invoke_syscall(SYSCALL_FS_FOPEN, path, mode, 0, 0, 0);    // return the id of file descriptor
}

int sys_fread(int fd, char *buff, int length)
{
    // TODO [P6-task2]: Implement sys_fread
    return invoke_syscall(SYSCALL_FS_FREAD, fd, buff, length, 0, 0);  // return the length of trully read data
}

int sys_fwrite(int fd, char *buff, int length)
{
    // TODO [P6-task2]: Implement sys_fwrite
    return invoke_syscall(SYSCALL_FS_FWRITE, fd, buff, length, 0, 0);  // return the length of trully written data
}

int sys_fclose(int fd)
{
    // TODO [P6-task2]: Implement sys_fclose
    return invoke_syscall(SYSCALL_FS_FCLOSE, fd, 0, 0, 0, 0);  // sys_fclose succeeds
}

int sys_ln(char *src_path, char *dst_path)
{
    // TODO [P6-task2]: Implement sys_ln
    return invoke_syscall(SYSCALL_FS_LN, src_path, dst_path, 0, 0, 0);;  // sys_ln succeeds 
}

int sys_rm(char *path)
{
    // TODO [P6-task2]: Implement sys_rm
    return invoke_syscall(SYSCALL_FS_RM, path, 0, 0, 0, 0); // sys_rm succeeds 
}

int sys_lseek(int fd, int offset, int whence)
{
    // TODO [P6-task2]: Implement sys_lseek
    return invoke_syscall(SYSCALL_FS_LSEEK, fd, offset, whence, 0, 0);  // the resulting offset location from the beginning of the file
}
