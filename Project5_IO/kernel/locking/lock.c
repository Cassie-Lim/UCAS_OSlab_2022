#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <atomic.h>
#include <assert.h>
#include <os/string.h>
//--------------------------------------------Lock Interface------------------------------------
void init_locks(void)
{
    /* TODO: [p2-task2] initialize mlocks */
    for(int i=0; i<LOCK_NUM; i++){
        spin_lock_init(&mlocks[i].lock);
        mlocks[i].block_queue.prev = mlocks[i].block_queue.next = & mlocks[i].block_queue;    // initialize block_queue
        mlocks[i].key = 0;
        mlocks[i].pid = 0;
        mlocks[i].usage = UNUSED;
    }
}

void spin_lock_init(spin_lock_t *lock)
{
    /* TODO: [p2-task2] initialize spin lock */
    lock->status = UNLOCKED;
}

int spin_lock_try_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] try to acquire spin lock */
    return (atomic_swap(LOCKED, &lock->status)==UNLOCKED);
}

void spin_lock_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] acquire spin lock */
    while(atomic_swap(LOCKED, &lock->status)==LOCKED);
}

void spin_lock_release(spin_lock_t *lock)
{
    /* TODO: [p2-task2] release spin lock */
    lock->status = UNLOCKED;
}

int do_mutex_lock_init(int key)
{
    /* TODO: [p2-task2] initialize mutex lock */
    // 检查对应key是否已经分配锁
    for(int i=0;i<LOCK_NUM;i++){
        if(mlocks[i].usage == USING &&  mlocks[i].key == key)
            return i;
    }
    // 寻找空闲锁
    for(int i=0; i<LOCK_NUM; i++){
        if(mlocks[i].usage == UNUSED){
            mlocks[i].usage=USING;
            mlocks[i].key = key;
            return i;
        }
    }
    return -1;  // 无空闲锁
}

void do_mutex_lock_acquire(int mlock_idx)
{
    /* TODO: [p2-task2] acquire mutex lock */
    // 成功获得锁
    if(spin_lock_try_acquire(&mlocks[mlock_idx].lock)){
        mlocks[mlock_idx].pid = current_running[cpu_id]->pid;
        return;
    }
    // 获取锁失败
    do_block(&current_running[cpu_id]->list, &mlocks[mlock_idx].block_queue);
}

void do_mutex_lock_release(int mlock_idx)
{
    /* TODO: [p2-task2] release mutex lock */
    list_node_t* p, *head;
    head = &mlocks[mlock_idx].block_queue;
    p = head->next;
    // 阻塞队列为空，释放锁
    if(p==head){
        spin_lock_release(&mlocks[mlock_idx].lock);
    }
    else{
        mlocks[mlock_idx].pid = get_pcb_from_node(p)->pid;
        do_unblock(p);
    }
}

//--------------------------------------------Barrier Interface------------------------------------
void init_barriers(void){
    for(int i=0; i <BARRIER_NUM; i++){
        barrs[i].goal=0;
        barrs[i].wait_num=0;
        barrs[i].usage=UNUSED;
        barrs[i].wait_list.prev = barrs[i].wait_list.next = &barrs[i].wait_list; 
    }
}
int do_barrier_init(int key, int goal){
    // 寻找对应key是否已经有对应屏障变量
    for(int i=0; i<BARRIER_NUM; i++){
        if(barrs[i].usage==USING && barrs[i].key==key){ // 找到匹配屏障变量
            barrs[i].goal = goal;
            return i;
        }
    }
    // 寻找空闲屏障变量
    for(int i=0; i<BARRIER_NUM; i++){
        if(barrs[i].usage==UNUSED){ // 找到空闲屏障变量
            barrs[i].key = key;
            barrs[i].goal = goal;
            return i;
        }
    }
    return -1;  // 未找到，返回-1
}
void do_barrier_wait(int bar_idx){
    barrs[bar_idx].wait_num++;
    if(barrs[bar_idx].goal != barrs[bar_idx].wait_num)
        do_block(&current_running[cpu_id]->list, &barrs[bar_idx].wait_list);
    else{
        free_block_list(&barrs[bar_idx].wait_list);
        barrs[bar_idx].wait_num=0;
    }
}
void do_barrier_destroy(int bar_idx){
    free_block_list(&barrs[bar_idx].wait_list);
    barrs[bar_idx].key=0;
    barrs[bar_idx].goal=0;
    barrs[bar_idx].usage=UNUSED;
}

condition_t conds[CONDITION_NUM];

//--------------------------------------------Condition Interface------------------------------------
void init_conditions(void){
    for(int i=0; i<CONDITION_NUM; i++){
        conds[i].key = 0; 
        conds[i].usage = UNUSED;
        conds[i].wait_list.prev = conds[i].wait_list.next = &conds[i].wait_list; // 初始化等待队列
    }
}
int do_condition_init(int key){ 
    // 寻找对应key是否已经有对应条件变量
    for(int i=0; i<CONDITION_NUM; i++){
        if(conds[i].usage==USING && conds[i].key==key){ // 找到匹配条件变量
            return i;
        }
    }
    // 寻找空闲屏障变量
    for(int i=0; i<CONDITION_NUM; i++){
        if(conds[i].usage == UNUSED){ // 找到空闲条件变量
            conds[i].key = key;
            return i;
        }
    }
    return -1;  // 未找到，返回-1
}
void do_condition_wait(int cond_idx, int mutex_idx){
    // 检查当前进程是否持有锁
    assert(mlocks[mutex_idx].lock.status==LOCKED && mlocks[mutex_idx].pid == current_running[cpu_id]->pid);   
    // 阻塞在条件变量的等待队列
    current_running[cpu_id]->status = TASK_BLOCKED;
    add_node_to_q(&current_running[cpu_id]->list, &conds[cond_idx].wait_list);
    do_mutex_lock_release(mutex_idx);   // 不能简单地调用do_block，因为其还需释放锁，而do_block自动调用了调度器
    do_scheduler();

}
void do_condition_signal(int cond_idx){
    list_node_t* head, *p;
    head = & conds[cond_idx].wait_list;
    p = head->next;
    if(p!=head)
        do_unblock(p);
}
void do_condition_broadcast(int cond_idx){
    free_block_list(&conds[cond_idx].wait_list);
}
void do_condition_destroy(int cond_idx){
    do_condition_broadcast(cond_idx);
    conds[cond_idx].key = 0;    
    conds[cond_idx].usage = UNUSED;    // 置为空闲
}

//--------------------------------------------Mailbox Interface------------------------------------
void init_mbox(){
    for(int i=0; i<MBOX_NUM; i++){
        mbox[i].name[0]='\0'; 
        mbox[i].wcur = 0;
        mbox[i].rcur = 0;
        mbox[i].user_num = 0;
        mbox[i].wait_mbox_full.prev = mbox[i].wait_mbox_full.next = &mbox[i].wait_mbox_full; 
        mbox[i].wait_mbox_empty.prev = mbox[i].wait_mbox_empty.next = &mbox[i].wait_mbox_empty; 
        // mbox[i].status = CLOSED;
    }    
}
int do_mbox_open(char *name){
    // 寻找对应name是否已经有对应邮箱
    for(int i=0; i<MBOX_NUM; i++){
        if(strcmp(mbox[i].name, name)==0){ // 找到匹配条件变量
            mbox[i].user_num ++;
            // mbox[i].status = OPEN;
            return i;
        }
    }
    // 寻找空闲邮箱变量
    for(int i=0; i<MBOX_NUM; i++){
        if(mbox[i].name[0]=='\0'){
            strcpy(mbox[i].name, name);
            mbox[i].user_num ++;
            // mbox[i].status = OPEN; 
            return i;
        }
    }
    return -1;  // 未找到，返回-1    
}
void do_mbox_close(int mbox_idx){
    mbox[mbox_idx].user_num--;
    if(mbox[mbox_idx].user_num==0){
        // mbox[i].status = CLOSED;
        mbox[mbox_idx].name[0] = '\0';
        mbox[mbox_idx].wcur = 0;
        mbox[mbox_idx].rcur = 0;
    }
}
#define MODE_W 0
#define MODE_R 1
void myMemcpy(char *dest, char *src, int vcur, int len, int arr_size, int mode){
    int pcur;   //cur的实际值
    // 写操作中dest是循环数组
    if(mode==MODE_W){
        for(int i=0; i<len; i++){
            pcur = (i + vcur) % arr_size;
            dest[pcur] = src[i];
        }
    }
    // 读操作中src是循环数组
    else{
        for(int i=0; i<len; i++){
            pcur = (i + vcur) % arr_size;
            dest[i] = src[pcur];
        }
    }
}
// int do_mbox_send(int mbox_idx, void * msg, int msg_length){
//     int len = msg_length;
//     int tmp_wcur;
//     // 邮箱已满，阻塞
//     while((tmp_wcur= mbox[mbox_idx].wcur + len)>MAX_MBOX_LENGTH + mbox[mbox_idx].rcur){
//         // 拷贝部分数据
//         len = MAX_MBOX_LENGTH + mbox[mbox_idx].rcur - mbox[mbox_idx].wcur;
//         myMemcpy(mbox[mbox_idx].msg, msg, mbox[mbox_idx].wcur, len, MAX_MBOX_LENGTH, MODE_W);
//         mbox[mbox_idx].wcur = MAX_MBOX_LENGTH  + mbox[mbox_idx].rcur;
//         msg += len;
//         free_block_list(&mbox[mbox_idx].wait_mbox_empty);   // 唤醒所有因邮箱空而阻塞的进程
//         do_block(&current_running[cpu_id]->list, &mbox[mbox_idx].wait_mbox_full);
//     }
//     // 进行数据拷贝
//     myMemcpy(mbox[mbox_idx].msg, msg, mbox[mbox_idx].wcur, len, MAX_MBOX_LENGTH, MODE_W);
//     mbox[mbox_idx].wcur = tmp_wcur;
//     free_block_list(&mbox[mbox_idx].wait_mbox_empty);   // 唤醒所有因邮箱空而阻塞的进程
//     return 0;
// }
// int do_mbox_recv(int mbox_idx, void * msg, int msg_length){
//     int len = msg_length;
//     int tmp_rcur;
//     // 邮箱读空，阻塞
//     while((tmp_rcur = mbox[mbox_idx].rcur - len)< mbox[mbox_idx].wcur){
//         len = mbox[mbox_idx].wcur - mbox[mbox_idx].rcur;
//         mbox[mbox_idx].rcur = mbox[mbox_idx].wcur;
//         myMemcpy(msg, mbox[mbox_idx].msg, mbox[mbox_idx].rcur, len, MAX_MBOX_LENGTH, MODE_R);
//         msg += len;
//         free_block_list(&mbox[mbox_idx].wait_mbox_full);   // 唤醒所有因邮箱满而阻塞的进程
//         do_block(&current_running[cpu_id]->list, &mbox[mbox_idx].wait_mbox_empty);
//     }
//     // 进行数据拷贝
//     mbox[mbox_idx].rcur = tmp_rcur;
//     myMemcpy(msg, mbox[mbox_idx].msg, mbox[mbox_idx].rcur, len, MAX_MBOX_LENGTH, MODE_R); // 注意rcur指向的是下一个空位
//     free_block_list(&mbox[mbox_idx].wait_mbox_full);   // 唤醒所有因邮箱满而阻塞的进程
//     return 0;
// }
int do_mbox_send(int mbox_idx, void * msg, int msg_length){
    int tmp_wcur;
    int cnt=0;
    // 邮箱已满，阻塞
    while((tmp_wcur= mbox[mbox_idx].wcur + msg_length)>MAX_MBOX_LENGTH + mbox[mbox_idx].rcur){
        do_block(&current_running[cpu_id]->list, &mbox[mbox_idx].wait_mbox_full);
        cnt++;
    }
    // 进行数据拷贝
    myMemcpy(mbox[mbox_idx].msg, msg, mbox[mbox_idx].wcur, msg_length, MAX_MBOX_LENGTH, MODE_W);
    mbox[mbox_idx].wcur = tmp_wcur;
    free_block_list(&mbox[mbox_idx].wait_mbox_empty);   // 唤醒所有因邮箱空而阻塞的进程
    return cnt;
}
int do_mbox_recv(int mbox_idx, void * msg, int msg_length){
    int tmp_rcur;
    int cnt=0;
    // 邮箱读空，阻塞
    while((tmp_rcur = mbox[mbox_idx].rcur + msg_length) > mbox[mbox_idx].wcur){
        do_block(&current_running[cpu_id]->list, &mbox[mbox_idx].wait_mbox_empty);
        cnt++;
    }
    // 进行数据拷贝
    myMemcpy(msg, mbox[mbox_idx].msg, mbox[mbox_idx].rcur, msg_length, MAX_MBOX_LENGTH, MODE_R); // 注意rcur指向的是下一个空位
    mbox[mbox_idx].rcur = tmp_rcur;
    free_block_list(&mbox[mbox_idx].wait_mbox_full);   // 唤醒所有因邮箱满而阻塞的进程
    return cnt;
}