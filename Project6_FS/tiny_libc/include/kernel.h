#ifndef __INCLUDE_KERNEL_H__
#define __INCLUDE_KERNEL_H__

#include <stdint.h>
#define NO_SYS_YIELD
#define KERNEL_JMPTAB_BASE 0x51ffff00
typedef enum {
    CONSOLE_PUTSTR,
    CONSOLE_PUTCHAR,
    CONSOLE_GETCHAR,
    MOVE_CURSOR = 7,
    PRINT,
    YIELD,
    MUTEX_INIT,
    MUTEX_ACQ,
    MUTEX_RELEASE,
    NUM_ENTRIES
} jmptab_idx_t;

static inline long call_jmptab(long which, long arg0, long arg1, long arg2)
{
    unsigned long val = \
        *(unsigned long *)(KERNEL_JMPTAB_BASE + sizeof(unsigned long) * which);
    long (*func)(long, long, long) = (long (*)(long, long, long))val;

    return func(arg0, arg1, arg2);
}

static inline void bios_putstr(char *str)
{
    call_jmptab(CONSOLE_PUTSTR, (long)str, 0, 0);
}

static inline void bios_putchar(int ch)
{
    call_jmptab(CONSOLE_PUTCHAR, (long)ch, 0, 0);
}

static inline int bios_getchar(void)
{
    return call_jmptab(CONSOLE_GETCHAR, 0, 0, 0);
}

static inline void kernel_move_cursor(int x, int y)
{
    call_jmptab(MOVE_CURSOR, (long)x, (long)y, 0);
}

static inline void kernel_print(char *fmt, long arg0, long arg1)
{
    call_jmptab(PRINT, (long)fmt, arg0, arg1);
}

static inline void kernel_yield(void)
{
    call_jmptab(YIELD, 0, 0, 0);
}

static inline int kernel_mutex_init(int key)
{
    return call_jmptab(MUTEX_INIT, (long)key, 0, 0);
}

static inline void kernel_mutex_acquire(int mlock_idx)
{
    call_jmptab(MUTEX_ACQ, (long)mlock_idx, 0, 0);
}

static inline void kernel_mutex_release(int mlock_idx)
{
    call_jmptab(MUTEX_RELEASE, (long)mlock_idx, 0, 0);
}

#endif