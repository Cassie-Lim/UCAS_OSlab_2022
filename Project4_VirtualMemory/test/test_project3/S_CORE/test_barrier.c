#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

int main(int argc, int arg0, int arg1)
{
    assert(argc >= 2);
    int print_location = arg0;
    int handle = arg1;    

    // Set random seed
    srand(clock());

    // Enter and exit target barrier
    for (int i = 0; i < 10; ++i)
    {
        int sleep_time = rand() % 3 + 1;
        sys_move_cursor(0, print_location);
        printf("> [TASK] Ready to enter the barrier.(%d) ", i);

        sys_barrier_wait(handle);

        sys_move_cursor(0, print_location);
        printf("> [TASK] Exited barrier (%d).(sleep %d s)",
               i, sleep_time);

        sys_sleep(sleep_time);
    }

    return 0;
}