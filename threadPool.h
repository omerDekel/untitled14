#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <sys/types.h>
#include "osqueue.h"
typedef struct thread_pool
{
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t* threads;
    int stopped;
    int numOfThreads;
    struct os_queue* tasks;
 int insertTasks;
}ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
