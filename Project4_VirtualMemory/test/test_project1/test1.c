#include <bios.h>

#define BUF_LEN 50

char buf[BUF_LEN];

int main(void)
{
    bios_putstr("[test1 ] Info: passed test1 check!\n");
    return 0;
}