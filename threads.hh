#if !defined(THREADS_HH)
#define THREADS_HH

#include <pthread.h>
#include <thread>

#include "creature.hh"

#define MAXTHREADS 8

//kottkech17 & leejeung

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

taskQueue_t * q;

void initTaskQueue(){
  q = (taskQueue_t *)malloc(sizeof(taskQueue_t));
  pthread_mutex_init(&q->lock, NULL);
  
  q->head = NULL;
  q->tail = NULL;

  for(int i = 0; i < MAXTHREADS; ++i){
    t[i] = std::thread(queueRun);
  }
}

void queueRun(){
  taskNode_t * node = NULL;
  while(1){
    pthread_mutex_lock(&q->lock);

    //If the queue is empty
    if(q->head == NULL){
      node = NULL;
    }
    else{
      node = q->head;
      q->head = q->head->next;

      if(q->head == NULL){
        q->tail = NULL;
      }
    }
    
    pthread_mutex_unlock(&q->lock);

    if(node != NULL){
      node->task(node->c); //Run task;
    }
  }
}

void addTask(void(*task)(creature*), creature * c){
  taskNode_t * node = (taskNode_t *)malloc(sizeof(taskNode_t));
  node->task = task;
  node->c = c;
  node->next = NULL;

  pthread_mutex_lock(&q->lock);

  if(q->tail != NULL){
    q->tail->next = node;
  }
  q->tail = node;

  if(q->head == NULL){
    q->head = node;
  }

  pthread_mutex_unlock(&q->lock);
}

#endif
