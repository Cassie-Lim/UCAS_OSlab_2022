#include <common.h>
#include <asm/biosdef.h>

#define BIOS_FUNC_ENTRY 0x50150000
#define IGNORE 0

static long call_bios(long which, long arg0, long arg1, long arg2, long arg3, long arg4)
{
    long (*bios_func)(long,long,long,long,long,long) = \
        (long (*)(long,long,long,long,long,long))BIOS_FUNC_ENTRY;

    return bios_func(which, arg0, arg1, arg2, arg3, arg4);
}

void port_write_ch(char ch)
{
    call_bios((long)BIOS_PUTCHAR, (long)ch, IGNORE, IGNORE, IGNORE, IGNORE);
}

void port_write(char *str)
{
    call_bios((long)BIOS_PUTSTR, (long)str, IGNORE, IGNORE, IGNORE, IGNORE);
}

int port_read_ch(void)
{
    return call_bios((long)BIOS_GETCHAR, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sd_read(unsigned mem_address, unsigned num_of_blocks, unsigned block_id)
{
    return (int)call_bios((long)BIOS_SDREAD, (long)mem_address, \
                            (long)num_of_blocks, (long)block_id, IGNORE, IGNORE);
}