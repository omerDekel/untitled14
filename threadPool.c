#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include "threadPool.h"
#include "osqueue.h"
typedef struct {
    void* param;
    void (*computeFunc)(void * param);
} Task;
void* executeTasks(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    struct os_queue* tasksQ;
    tasksQ = pool->tasks;
    //printf("here\n");

    while (!pool->stopped ) {
        pthread_mutex_lock(&(pool->lock));
        if (osIsQueueEmpty(tasksQ)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
            //printf("sent cond signal\n");
        }

            Task *task = osDequeue(pool->tasks);
            if (task == NULL) {
                printf("ERROR\n");
            }
            pthread_mutex_unlock(&(pool->lock));

            task->computeFunc(task->param);
    }
    printf("stopppp\n");

}
ThreadPool* tpCreate(int numOfThreads) {
    int j;
    ThreadPool* threadPool;
    threadPool= (ThreadPool *)malloc(sizeof(ThreadPool));
    threadPool->threads = (pthread_t *)malloc(numOfThreads*sizeof(pthread_t));
    threadPool->numOfThreads= numOfThreads;
    threadPool->tasks = osCreateQueue();
    threadPool->stopped = 0;
    threadPool->insertTasks = 1;
    pthread_mutex_init(&(threadPool->lock),NULL);
    pthread_cond_init(&(threadPool->notify),NULL);
    for ( j = 0; j < numOfThreads; ++j) {
        pthread_create(&(threadPool->threads[j]),NULL,executeTasks,(void *)threadPool);
    }
    return threadPool;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks){
    if (threadPool->stopped == 1) {
        return;
    }
    int i,err;
    if  (shouldWaitForTasks == 0) {
        threadPool->stopped = 1;
        return;
    }
    threadPool->insertTasks = 0;
    //printf("broad\n");
    //pthread_cond_broadcast(&(threadPool->notify));
    for (i = 0; i < threadPool->numOfThreads; i++) {
        //printf("hayushhh\n");
        err = pthread_join(threadPool->threads[i], NULL);
        if (err != 0) {
            printf("Failure: waiting for thread no. %d\n", i);
        }
    }
    pthread_mutex_lock(&threadPool->lock);
    threadPool->stopped = 1;
    pthread_mutex_unlock(&threadPool->lock);
    return;
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    Task *t = (Task *)malloc(sizeof(Task));
    if (t == NULL || threadPool->stopped == 1 || threadPool->insertTasks == 0) {
        printf("ERROR!!\n");
        return  -1;
    }
    t->param = param;
    t->computeFunc = computeFunc;
    osEnqueue(threadPool->tasks , (void *)t);
    if (pthread_cond_signal(&(threadPool->notify)) !=0) {
        printf("couldn't send cond signal");
    }
}