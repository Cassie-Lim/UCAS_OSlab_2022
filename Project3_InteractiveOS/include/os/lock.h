/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                                   Thread Lock
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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

#ifndef INCLUDE_LOCK_H_
#define INCLUDE_LOCK_H_

#include <os/list.h>

//--------------------------------------------Lock Interface------------------------------------
#define LOCK_NUM 16
typedef enum {
    UNLOCKED,
    LOCKED,
} lock_status_t;

typedef enum {
    UNUSED,
    USING,
} use_status_t;
typedef struct spin_lock
{
    volatile lock_status_t status;
} spin_lock_t;

typedef struct mutex_lock
{
    spin_lock_t lock;
    list_head block_queue;
    int key;
    int pid;    //记录持有者
    use_status_t usage; // 记录是否被使用
} mutex_lock_t;

mutex_lock_t mlocks[LOCK_NUM];

void init_locks(void);

void spin_lock_init(spin_lock_t *lock);
int spin_lock_try_acquire(spin_lock_t *lock);
void spin_lock_acquire(spin_lock_t *lock);
void spin_lock_release(spin_lock_t *lock);

int do_mutex_lock_init(int key);
void do_mutex_lock_acquire(int mlock_idx);
void do_mutex_lock_release(int mlock_idx);

//--------------------------------------------Barrier Interface------------------------------------
typedef struct barrier
{
    // TODO [P3-TASK2 barrier]
    int goal;
    int wait_num;
    list_head wait_list;
    int key;
    use_status_t usage; // 记录是否被使用
} barrier_t;

#define BARRIER_NUM 16
barrier_t barrs[BARRIER_NUM];

void init_barriers(void);
int do_barrier_init(int key, int goal);
void do_barrier_wait(int bar_idx);
void do_barrier_destroy(int bar_idx);

//--------------------------------------------Condition Interface------------------------------------
typedef struct condition
{
    // TODO [P3-TASK2 condition]
    list_head wait_list;
    int key;
    use_status_t usage; // 记录是否被使用
} condition_t;

#define CONDITION_NUM 16

void init_conditions(void);
int do_condition_init(int key);
void do_condition_wait(int cond_idx, int mutex_idx);
void do_condition_signal(int cond_idx);
void do_condition_broadcast(int cond_idx);
void do_condition_destroy(int cond_idx);

//--------------------------------------------Mbox Interface------------------------------------
#define MAX_MBOX_LENGTH (64)
#define NAME_LEN 20
// typedef enum {
//     CLOSED,
//     OPEN,
// } mbox_status_t;
typedef struct mailbox
{
    // TODO [P3-TASK2 mailbox]
    char name[NAME_LEN];
    char msg[MAX_MBOX_LENGTH+1];
    int wcur;    // 写指针，指向首个空闲块
    int rcur;   // 读指针，记录下一个要读的位置
    int user_num;    // 当前使用数
    // list_head wait_list;
    list_head wait_mbox_full;   // 由于邮箱满被阻塞的进程
    list_head wait_mbox_empty;  // 由于邮箱空被阻塞的进程
    // mbox_status_t status;   // 是否被打开
} mailbox_t;
// 邮箱空：wcur=rcur，满：wcur=rcur+MAX_MBOX_LENGTH
#define MBOX_NUM 16
mailbox_t mbox[MBOX_NUM];

void init_mbox();
int do_mbox_open(char *name);
void do_mbox_close(int mbox_idx);
int do_mbox_send(int mbox_idx, void * msg, int msg_length);
int do_mbox_recv(int mbox_idx, void * msg, int msg_length);
#endif
