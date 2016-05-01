#if !defined(THREADS_HH)
#define THREADS_HH

#include <pthread.h>
#include <thread>

#include "creature.hh"

#define MAXTHREADS 8

//Struct for arbitrary tasks to be fed into the thread pool
typedef struct taskNode{
  taskNode * next;
  void (*task)(creature* c);
  creature * c;
}taskNode_t;

//The structure for the task Queue itself
typedef struct taskQueue{
  pthread_mutex_t lock;
  taskNode_t * head;
  taskNode_t * tail;
}taskQueue_t;

// Initializes the task queue
void initTaskQueue(taskQueue * q);

std::thread t[MAXTHREADS];

void initTaskQueue(taskQueue * q){
  q = (taskQueue_t *)malloc(sizeof(taskQueue_t));
  pthread_mutex_init(&q->lock, NULL);
  
  q->head = NULL;
  q->tail = NULL;

  /*for(int i = 0; i < MAXTHREADS; ++i){
    t[i] = std::thread(queueRun);
    }*/
}

#endif
