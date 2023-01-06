#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
// 包格式
#define SRC_START  6
#define DEST_START 0
#define DATA_START 54
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

        int print_location = 2;
        sys_move_cursor(0, print_location);
        printf("[SEND] Round %d, start send(%d): ", r+1, buff_num);

        char* curr = buffer;
        for(int i=0; i<buff_num;){
            // 还没有包到来
            if(package_len[i]==0)
                continue;
            else{
                // 将原本的目的MAC地址用作新的源MAC地址
                for(int i=0; i<6; i++)
                    curr[SRC_START+i] = curr[DEST_START+i];
                // 将目的MAC地址修改为广播MAC地址
                for(int i=DEST_START; i<DEST_START+6; i++)
                    curr[i] = 0xff;
                // 将数据"Requests"为"Response"
                curr[DATA_START+2] = 's';
                curr[DATA_START+3] = 'p';
                curr[DATA_START+4] = 'o';
                curr[DATA_START+5] = 'n';
                curr[DATA_START+6] = 's';
                curr[DATA_START+7] = 'e';
                sys_net_send(curr, package_len[i]);
                sys_move_cursor(0, print_location + 1);
                printf("[SEND] replying %02d/%02d\n", i+1, buff_num);
                for (int j = 0; j < (package_len[i] + 15) / 16; ++j) {
                    for (int k = 0; k < 16 && (j * 16 + k < package_len[i]); ++k) {
                        printf("%02x ", (uint32_t)(*(uint8_t*)curr));
                        ++curr;
                    }
                    printf("\n");
                }
                i++;
            }
        }
    }
    return 0;
}