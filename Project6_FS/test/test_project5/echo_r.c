#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


int main(int argc, char* argv[])
{
    assert(argc>=5);
    uint32_t key = atoi(argv[1]);
    int buff_len = atoi(argv[2]);
    int buff_num = atoi(argv[3]);
    int round = atoi(argv[4]);

    uint64_t shmpage[round];
    for(int r=0; r<round; r++){
        shmpage[r] = sys_shmpageget(key+r); 
        char *buffer = (char*)shmpage[r];
        int* package_len = (int*)(shmpage[r] + buff_len*buff_num*sizeof(char));

        int print_location = 1;
        sys_move_cursor(0, print_location);
        printf("[RECV] Round %d, start recv(%d): ", r+1, buff_num);

        int ret = sys_net_recv(buffer, buff_num, package_len);

        sys_move_cursor(0, print_location);
        printf("[RECV] Round %d, recv %d bytes, %d packages", r+1, ret, buff_num);
    }
    return 0;
}