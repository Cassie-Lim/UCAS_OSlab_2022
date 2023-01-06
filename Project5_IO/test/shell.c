/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   50

#define SHELL_BEGIN 20
#define BUF_LEN 50
#define ARG_LEN 20
#define MAX_ARG_NUM 6    //不超过六个参数（含命令）
static char buffer[BUF_LEN];
char* args_ptr[MAX_ARG_NUM];
char args[MAX_ARG_NUM][ARG_LEN];  
int buff_end(int cur);                            // 判断buffer是否已经到结尾
static inline int parse_op(const char* buffer);   // 解析buffer中存储的指令，并将解析参数存于args，返回参数个数

static inline void shell_clear();                 // clear命令对应函数
static inline void shell_exec(int argc);          // exec命令对应函数
static inline void shell_kill();                  // kill命令对应函数
static inline void shell_ps();                    // ps命令对应函数
static inline void shell_waitpid();               // waitpid命令对应函数
static inline void shell_taskset();               // taskset命令对应函数
static inline void shell_unknown();               // 未知命令对应函数

int main(void)
{
    shell_clear();
    printf("> root@UCAS_OS: ");
    int index=0, j, k, tmp, arg_num;
    // 初始化args_ptr
    for(int i=0; i<MAX_ARG_NUM; i++)
        args_ptr[i] = args+i;
    while (1)
    {
        // TODO [P3-task1]: call syscall to read UART port
        while((tmp = sys_getchar())==-1);
        if(index>0 || !(tmp=='\b'|| tmp==127))  // 避免删除命令输入提示符
            sys_putchar(tmp);
        // TODO [P3-task1]: parse input
        // note: backspace maybe 8('\b') or 127(delete)
        if(tmp=='\b'|| tmp==127){
            if(index>0)
                index--;
            buffer[index]=0;
        }
        // TODO [P3-task1]: ps, exec, kill, clear
        else if(tmp=='\n' || tmp=='\r'){
            if(index==0){ // buffer空，只需要换行
                sys_putchar('\n');
                continue;
            }
            buffer[index]=tmp;
            arg_num = parse_op(buffer);
            if(strcmp("ps", args[0])==0)
                shell_ps();
            else if(strcmp("exec", args[0])==0)
                shell_exec(arg_num-1);
            else if(strcmp("kill", args[0])==0)
                shell_kill();
            else if(strcmp("clear", args[0])==0)
                shell_clear();
            else if(strcmp("waitpid", args[0])==0)
                shell_waitpid();
            else if(strcmp("taskset", args[0])==0)
                shell_taskset();
            else
                shell_unknown();
            // index清零
            index = 0;
            // 清空buffer
            for(j=0; j<BUF_LEN; j++)
                buffer[j]=0;
            // 清空args
            for(j=0; j<MAX_ARG_NUM; j++)
                for(k=0; k<ARG_LEN; k++)
                    args[j][k]=0;
            // 打印命令输入行
            printf("> root@UCAS_OS: ");
        }        
        else
            buffer[index++]=tmp;
    }
    return 0;
}
int buff_end(int cur){
    return cur>=BUF_LEN || buffer[cur]=='\n' || buffer[cur]=='\r' || buffer[cur]=='\0';
}
static inline int parse_op(const char * buffer){ // 解析buffer中存储的指令，并将解析参数存于args
    int cur=0, i, j;
    while(!buff_end(cur) && isspace(buffer[cur])) //跳过空格
        cur++;
    for(i=0;i<MAX_ARG_NUM;i++){
        j=0;
        while(!buff_end(cur) && !isspace(buffer[cur])) {   //遇到首个空格则停止copy
            args[i][j++]=buffer[cur++];
        }
        args[i][j]='\0';
        while(!buff_end(cur) && isspace(buffer[cur])) //跳过空格
            cur++;
        if(buff_end(cur))    
            return ++i;
    }
    return i;
}

static inline void shell_clear(){
    sys_clear();
    sys_move_cursor(0, SHELL_BEGIN);
    printf("------------------- COMMAND -------------------\n");
}

static inline void shell_exec(int argc){
    int do_wait = strcmp(args[argc], "&"); //结尾不为&需要等待，若传参NULL，默认无需等待
    pid_t pid = sys_exec(args[1], argc- (do_wait?0:1), args_ptr+1);     
    if(pid==0){
        printf("Error: exec failed!\n");
    }
    else{
        printf("Info: excute %s successfully, pid = %d\n", args[1], pid);
        if(do_wait)
            sys_waitpid(pid);
    }
}

static inline void shell_kill(){
    int pid = atoi(args[1]);
    if(sys_kill(pid)==0)
        printf("Info: Cannot find process with pid %d!\n", pid);
    else
        printf("Info: kill process %d successfully.\n", pid);
}

static inline void shell_ps(){
    sys_ps();
}

static inline void shell_waitpid(){
    int pid = atoi(args[1]);
    if(sys_waitpid(pid)==0)
        printf("Info: Cannot find process with pid %d!\n", pid);
    else
        printf("Info: Excute waitpid successfully, pid = %d.\n", pid);
}

static inline void shell_taskset(){
    int mode_p=0, pid=0, mask=0;
    if(strcmp(args[1], "-p")==0){
        mode_p = 1;
        mask = atoi(args[2]);
        pid = atoi(args[3]);
        sys_taskset(mode_p, mask, (void*)pid);
        printf("Info: Excute taskset successfully.\n");
    }
    else{
        mask = atoi(args[1]);
        pid = sys_taskset(mode_p, mask, (void*)args[2]);
        printf("Info: Excute taskset successfully, pid = %d.\n", pid);
    }
}

static inline void shell_unknown(){
    printf("Error: Unknown command %s\n", args[0]);
}