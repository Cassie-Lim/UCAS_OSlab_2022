#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define RW_START (1<<23)
#define SEEK_SET 0
static char buff[64];

int main(void)
{
    int fd = sys_fopen("1.txt", O_RDWR);
//------------------------------******REVISE START******-----------------------------------
    assert(fd>=0);
    assert(sys_lseek(fd, RW_START, SEEK_SET)==RW_START);
//------------------------------*******REVISE END*******-----------------------------------
    // write 'hello world!' * 10
    for (int i = 0; i < 10; i++)
    {
        sys_move_cursor(0, 0);
        sys_fwrite(fd, "hello world!\n", 13);
    }
    // read
    for (int i = 0; i < 10; i++)
    {
        sys_fread(fd, buff, 13);
        for (int j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    sys_fclose(fd);

    return 0;
}