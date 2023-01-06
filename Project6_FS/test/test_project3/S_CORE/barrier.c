#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define NUM_TB 3
#define BARR_KEY 58
#define BUF_LEN 20

int main(int argc, int arg0)
{
    char* argv[2];
    int print_location = (argc == 1) ? 0 : arg0;

    // Initialize barrier
    int handle = sys_barrier_init(BARR_KEY, NUM_TB);

    // Launch child processes
    pid_t pids[NUM_TB];
    
    argv[1] = (char*)handle;
    // Start three test programs
    for (int i = 0; i < NUM_TB; ++i)
    {
        // TODO: [P3-TASK2 S-core] use your "test_barrier" id here        
        int tb_id = -1;
        assert(tb_id != -1);
        argv[0] = (char*)(print_location + i);
        pids[i] = sys_exec(tb_id, 2, argv);       
        // pids[i] = sys_exec(tb_id, 2, print_location + i, handle, 0);       
    }

    // Wait child processes to exit
    for (int i = 0; i < NUM_TB; ++i)
    {
        sys_waitpid(pids[i]);
    }

    // Destroy barrier
    sys_barrier_destroy(handle);

    return 0;
}