#ifndef __INCLUDE_BIOS_H__
#define __INCLUDE_BIOS_H__

#define BIOS_JMPTAB_BASE 0x51ffff00
typedef enum {
    CONSOLE_PUTSTR,
    CONSOLE_PUTCHAR,
    CONSOLE_GETCHAR,
    SD_READ,
    NUM_ENTRIES
} jmptab_idx_t;

static inline long call_jmptab(long which, long arg0, long arg1, long arg2)
{
    unsigned long val = \
        *(unsigned long *)(BIOS_JMPTAB_BASE + sizeof(unsigned long) * which);
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

static inline int bios_sdread(unsigned mem_address, unsigned num_of_blocks, \
                              unsigned block_id)
{
    return call_jmptab(SD_READ, (long)mem_address, (long)num_of_blocks, \
                        (long)block_id);
}

#endif