#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#define DATA_SIZE 1000
static char blank[] = {"                                                                   "};
int data[DATA_SIZE];
int cur=0;
int sum=0;
int test(int id, int print_location){
    while(cur<DATA_SIZE){
        sum+=data[cur++];
        sys_move_cursor(0, print_location+id);
        printf("> [TASK] Thread %d is calculating...cur is %d\n", id, cur);
    }
    sys_move_cursor(0, print_location+id);
    printf("%s", blank);
    sys_move_cursor(0, print_location+id);
    printf("> [TASK] Thread %d finishes.\n", id);
    while(1);
}
int main(void)
{
    int print_location = 6;
    int args[][3] = {1, print_location, 0,
                    2, print_location, 0};
    for(int i=0;i<DATA_SIZE;i++){
        data[i] = i+1;
    }
    sys_move_cursor(0, print_location);
    printf("> [TASK] This task is to test thread create.\n");
    for(int i=0;i<2;i++){
        if(sys_thread_create((uint64_t)test, args[i])){
            sys_move_cursor(0, print_location);
            printf("Create thread %d failed!\n",i);
        }
    }
    sys_move_cursor(0, print_location);
    printf("%s", blank);
    sys_move_cursor(0, print_location);
    printf("> [TASK] Main thread is running...\n");
    while(cur<DATA_SIZE);
    sys_move_cursor(0, print_location);
    printf("%s", blank);
    sys_move_cursor(0, print_location);
    printf("> [TASK] The final result is %d.\n", sum);
    while (1);
}
