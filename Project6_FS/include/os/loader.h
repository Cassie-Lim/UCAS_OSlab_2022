#ifndef __INCLUDE_LOADER_H__
#define __INCLUDE_LOADER_H__

#include <type.h>

uint64_t load_task_img(char *taskname);
uint64_t map_task(char *taskname, uintptr_t pgdir);
// uint64_t load_task_img(int taskid);


#endif