#include <bios.h>

#define BUF_LEN 50

char buf[BUF_LEN];

int main(void)
{
    bios_putstr("[test2 ] Info: passed test2 check!\n");
    return 0;
}