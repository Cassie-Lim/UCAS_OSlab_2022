#include <stdio.h>
#include <unistd.h>
// This test is for testing recycle pcb and resources
int main(){
    for(int i=0; i<16; i++){
        pid_t pid = sys_exec("fly",0, 0);
        sys_kill(pid);
    }
}