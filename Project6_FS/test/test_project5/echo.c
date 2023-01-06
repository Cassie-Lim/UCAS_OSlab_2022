#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define ROUND 8
#define SHMP_KEY 48
#define BUFF_NUM 16
#define BUFF_SIZE 200
#define ARG_BUFF_LEN 10
/**
* share page arrangement:
* char buff[BUFF_NUM*BUFF_SIZE];
* int package_len[BUFF_NUM];
**/

int main(void)
{
    int print_location = 0;
    int iteration = 1;
    // 创建共享页面
    uint64_t shmpage[ROUND]; 
    for(int i=0; i<ROUND; i++){
        shmpage[i] = sys_shmpageget(SHMP_KEY+i);
    }
    int pid[2];
    char arg_buff1[ARG_BUFF_LEN];
    char arg_buff2[ARG_BUFF_LEN];
    char arg_buff3[ARG_BUFF_LEN];
    char arg_buff4[ARG_BUFF_LEN];
    assert(itoa(SHMP_KEY, arg_buff1, ARG_BUFF_LEN, 10) != -1);
    assert(itoa(BUFF_SIZE, arg_buff2, ARG_BUFF_LEN, 10) != -1);
    assert(itoa(BUFF_NUM, arg_buff3, ARG_BUFF_LEN, 10) != -1);
    assert(itoa(ROUND, arg_buff4, ARG_BUFF_LEN, 10) != -1);

    sys_move_cursor(0, print_location);
    printf("[ECHO] start send & recv");
    // 创建接收进程
    char* args_r[5] = {"echo_r", arg_buff1, arg_buff2, arg_buff3, arg_buff4};
    pid[0] = sys_exec(args_r[0], 5, args_r);
    // 创建发送进程
    char* args_s[5] = {"echo_s", arg_buff1, arg_buff2, arg_buff3, arg_buff4};
    pid[1] = sys_exec(args_s[0], 5, args_s);

    sys_waitpid(pid[0]);
    sys_waitpid(pid[1]);
    sys_move_cursor(0, print_location);
    printf("[ECHO] Finish send & recv and exit!");

    return 0;
}