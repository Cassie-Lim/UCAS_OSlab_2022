#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define LOCK1_KEY 20
#define LOCK2_KEY 22

#define BUF_LEN 10

int main(int argc, int arg0)
{
    char* argv[3];
    int print_location = (argc == 1) ? 0 : arg0;

    // Init two mutex locks, and convert decimal handle to string via itoa
    int handle1 = sys_mutex_init(LOCK1_KEY);
    int handle2 = sys_mutex_init(LOCK2_KEY);


    // Launch two assistant processes, and pass mutex handles to them
    // TODO: [P3-TASK1 S-core] use your "ready_to_exit" id here
    int rte_id = -1;
    assert(rte_id != -1);
    argv[0] = (char*)print_location + 1;
    argv[1] = (char*)handle1;
    argv[2] = (char*)handle2;
    pid_t pid1 = sys_exec(rte_id, 3, argv);
    // pid_t pid1 = sys_exec(rte_id, 3, print_location + 1, handle1, handle2);

    sys_sleep(1);  // wait enough time for task1 to acquire locks
    

    // TODO: [P3-TASK1 S-core] use your "wait_locks" id here
    int wl_id = -1;
    assert(wl_id != -1);
    argv[0] = (char*)print_location + 2;
    argv[1] = (char*)handle1;
    argv[2] = (char*)handle2;
    pid_t pid2 = sys_exec(wl_id, 3, argv);
    // pid_t pid2 = sys_exec(wl_id, 3, print_location + 2, handle1, handle2);

    // Start to waitpid(pid1)
    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=%d) to exit.", pid1);
    sys_waitpid(pid1);

    // Finish waitpid(pid1)
    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", pid1);

    // Start to waitpid(pid2)
    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=%d) to exit.", pid2);
    sys_waitpid(pid2);

    // Finish waitpid(pid2)
    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", pid2);

    return 0;
}