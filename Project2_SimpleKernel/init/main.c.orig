#include <common.h>
#include <asm.h>
#include <os/bios.h>
#include <os/task.h>
#include <os/string.h>
#include <os/loader.h>
#include <type.h>

#define VERSION_BUF 50
#define NBYTES2SEC(nbytes) (((nbytes) / SECTOR_SIZE) + ((nbytes) % SECTOR_SIZE != 0))
#define SECTOR_SIZE 512
int version = 2; // version must between 0 and 9
char buf[VERSION_BUF];

// Task info array
task_info_t tasks[TASK_MAXNUM];
char buffer [512];

static int bss_check(void)
{
    for (int i = 0; i < VERSION_BUF; ++i)
    {
        if (buf[i] != 0)
        {
            return 0;
        }
    }
    return 1;
}

static void init_bios(void)
{
    volatile long (*(*jmptab))() = (volatile long (*(*))())BIOS_JMPTAB_BASE;

    jmptab[CONSOLE_PUTSTR]  = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ]         = (long (*)())sd_read;
}

static void init_task_info(int app_info_loc, int app_info_size)
{

    // TODO: [p1-task4] Init 'tasks' array via reading app-info sector
    // NOTE: You need to get some related arguments from bootblock first
    int start_sec, blocknums;
    start_sec = app_info_loc / SECTOR_SIZE;
    blocknums = NBYTES2SEC(app_info_loc + app_info_size) - start_sec;
    bios_sdread((uint64_t)buffer, blocknums, start_sec);
    uint8_t *tmp = (uint8_t *)((uint64_t)buffer + app_info_loc -start_sec * SECTOR_SIZE);
    memcpy((uint8_t *)tasks, tmp, app_info_size);
    // for(int i=0; i<4; i++){
    //     bios_putchar(tasks[i].blocknums+'0');
    //     bios_putchar('\n');
    // }
}

int main(int app_info_loc, int app_info_size, int seq_end_loc, int seq_start_loc)
{
    // Check whether .bss section is set to zero
    int check = bss_check();

    // Init jump table provided by BIOS (ΦωΦ)
    init_bios();

    // Init task information (〃'▽'〃)
    init_task_info(app_info_loc, app_info_size);

    // Output 'Hello OS!', bss check result and OS version
    char output_str[] = "bss check: _ version: _\n\r";
    char output_val[2] = {0};
    int i, output_val_pos = 0;

    output_val[0] = check ? 't' : 'f';
    output_val[1] = version + '0';
    for (i = 0; i < sizeof(output_str); ++i)
    {
        buf[i] = output_str[i];
        if (buf[i] == '_')
        {
            buf[i] = output_val[output_val_pos++];
        }
    }

    bios_putstr("Hello OS!\n\r");
    bios_putstr(buf);

    // //读取终端输入并回显
    // int tmp;
    // while(1){
    //     while((tmp=bios_getchar())==-1);
    //     bios_putchar(tmp);
    // }
    
    // TODO: Load tasks by either task id [p1-task3] or task name [p1-task4],
    //   and then execute them.
    // [p1-task3]
    /**
    int taskid;
    uint64_t entry_addr;
    void (*entry) (void);
    while(1){
        while((taskid=bios_getchar())==-1);
        bios_putchar(taskid);
        taskid -= '0';
        if(taskid>=0 && taskid<=TASK_MAXNUM){
            bios_putchar('\n');
            entry_addr = load_task_img(taskid);
            entry = (void*) entry_addr;
            entry();
        }
    }
    **/
    // [p1-task4]
    /**
    char taskname[10] = "";
    char *str_tmp = "a";
    int tmp;
    uint64_t entry_addr;
    void (*entry) (void);
    while(1){
        while((tmp=bios_getchar())==-1);
        bios_putchar(tmp);
        if(tmp == '#'){
            bios_putchar('\n');
            entry_addr = load_task_img(taskname);
            if(entry_addr!=0){
                entry = (void*) entry_addr;
                entry();
            }
            taskname[0]='\0';
        }
        else{
            str_tmp[0]=tmp;
            strcat(taskname, str_tmp);
        }
        
    }
    **/
    // [p1-task5]
    /**
    // load task by id
    int start_sec = seq_start_loc / SECTOR_SIZE;
    int blocknums = NBYTES2SEC(seq_end_loc) - start_sec;
    uint64_t entry_addr;
    void (*entry) (void);
    bios_sdread(buffer, blocknums, start_sec);
    int taskid=0;
    for(int i= seq_start_loc - start_sec*SECTOR_SIZE; i< seq_end_loc - start_sec*SECTOR_SIZE; i++){
        if(buffer[i]=='#'){
            entry_addr=load_task_byid(taskid);
            if(entry_addr!=0){
                entry = (void*) entry_addr;
                entry();
            }
            taskid = 0;
        }
        else if(buffer[i]>='0' && buffer[i]<='9')
            taskid = taskid*10 + buffer[i] - '0';
    }
    **/
    // load task by name
    char taskname[10] = "";
    char *str_tmp = "a";
    int start_sec = seq_start_loc / SECTOR_SIZE;
    int blocknums = NBYTES2SEC(seq_end_loc) - start_sec;
    uint64_t entry_addr;
    void (*entry) (void);
    bios_sdread(buffer, blocknums, start_sec);
    int taskid=0;
    for(int i= seq_start_loc - start_sec*SECTOR_SIZE; i< seq_end_loc - start_sec*SECTOR_SIZE; i++){
        if(buffer[i]=='#'){
            // entry_addr=load_task_byid(taskid);
            entry_addr = load_task_img(taskname);
            if(entry_addr!=0){
                entry = (void*) entry_addr;
                entry();
            }
            taskname[0] = '\0';
        }
        else{
            str_tmp[0]=buffer[i];
            strcat(taskname, str_tmp);
        }
    }
    // Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
    while (1)
    {
        asm volatile("wfi");
    }

    return 0;
}
