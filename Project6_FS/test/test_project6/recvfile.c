#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define PACK_SIZE 512

static uint32_t header_buffer[PACK_SIZE];
static int header_length;
/**
[TODO] P6-Task3: 
    - 考虑发包未满的情况？
    - 考虑test文件在kernel内的处理方式：
        - 在task info数组里加上相关信息？
        - elf头是否要仿照createimage中处理？
**/
int main(int argc, char* argv[])
{
    assert(argc>=2);
    int print_location = 1;
    int iteration = 1;
    // ROUND1:接收elf文件; ROUND1:接收test文件
    for(int i=0; i<2; i++){
        sys_net_recv(header_buffer, 1, &header_length) == PACK_SIZE;
        // 获取接下来将发送的包数
        char num_str[17];
        memcpy(num_str, (char*)header_buffer, 16);
        num_str[16] = '\0';
        int num = atoi(num_str);
        // 创建buffer
        uint32_t recv_buffer[num * PACK_SIZE];
        int recv_length[num];   
        // 接受文件内容
        sys_move_cursor(0, print_location);
        printf("[RECV] start recv(%d): ", num);

        int ret = sys_net_recv(recv_buffer, num, recv_length);
        printf("%d, iteration = %d\n", ret, iteration++);
        char *curr = (char *)recv_buffer;
        for (int i = 0; i < num; ++i) {
            sys_move_cursor(0, print_location + 1);
            printf("packet %d:\n", i);
            for (int j = 0; j < (recv_length[i] + 15) / 16; ++j) {
                for (int k = 0; k < 16 && (j * 16 + k < recv_length[i]); ++k) {
                    printf("%02x ", (uint32_t)(*(uint8_t*)curr));
                    ++curr;
                }
                printf("\n");
            }
        }
        // 将内容写入文件
        char* file_name = argv[1];
        sys_touch(file_name);
        int fd = sys_fopen(file_name, O_RDWR);
        sys_fwrite(fd, recv_buffer, num * PACK_SIZE);
        sys_fclose(fd);
    }
    
    return 0;
}