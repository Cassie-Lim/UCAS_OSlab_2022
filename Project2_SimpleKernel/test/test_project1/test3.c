#include <bios.h>

#define BUF_LEN 50

char buf[BUF_LEN];

int main(void)
{
    bios_putstr("[test3 ] Info: passed test3 check!\n");
    return 0;
}