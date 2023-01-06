#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define NUM_TC 3
#define COND_KEY 58
#define LOCK_KEY 42
#define RESOURCE_ADDR 0x56000000
#define BUF_LEN 30

int main(int argc)
{
    assert(argc >= 1);
    // Launch child processes
    pid_t pids[NUM_TC + 1];
    char buf_location[BUF_LEN];

    assert(itoa(5, buf_location, BUF_LEN, 10) != -1);

    // Start server
    char *argv_server[2] = {"mbox_server", \
                              buf_location, \
                              };
    pids[0] = sys_exec(argv_server[0], 2, argv_server);

    // Start clients
    for (int i = 1; i < NUM_TC + 1; i++)
    {
        char *argv_client[2] = {"mbox_client", ""};
        pids[i] = sys_exec(argv_client[0], 1, argv_client);
    }

    // Wait produce processes to exit
    for (int i = 0; i < NUM_TC + 1; i++) {
        sys_waitpid(pids[i]);
    }

    return 0;    
}