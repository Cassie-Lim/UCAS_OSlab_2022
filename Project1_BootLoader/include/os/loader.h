#ifndef __INCLUDE_LOADER_H__
#define __INCLUDE_LOADER_H__

#include <type.h>

uint64_t load_task_img(char *taskname);
// uint64_t load_task_img(int taskid);
uint64_t load_task_byid(int taskid);

#endif