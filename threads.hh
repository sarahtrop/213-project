#if !defined(THREADS_HH)
#define THREADS_HH

#include <pthread.h>
#include <thread>

#include "creature.hh"

#define MAXTHREADS 8

//code structure from kottkech17 & leejeung galaxy lab

void queueRun();

//Struct for arbitrary tasks to be fed into the thread pool
typedef struct taskNode {
  taskNode * next;
  void (*task)(int i);
  int i;
} taskNode_t;

//The structure for the task Queue itself
typedef struct taskQueue {
  pthread_mutex_t lock;
  taskNode_t * head;
  taskNode_t * tail;
} taskQueue_t;

// Initializes the task queue
void initTaskQueue(taskQueue * q);

std::thread t[MAXTHREADS];

taskQueue_t * q;

pthread_cond_t taskCond;

pthread_mutex_t countTasks;
pthread_cond_t countCond;

int tasksFinished = 0;

void resetTasks(){
  pthread_mutex_lock(&countTasks);
  tasksFinished = 0;
  pthread_mutex_unlock(&countTasks);
}

void initTaskQueue(){
  q = (taskQueue_t *)malloc(sizeof(taskQueue_t));
  pthread_mutex_init(&q->lock, NULL);
  
  pthread_cond_init(&taskCond, NULL);

  pthread_cond_init(&countCond, NULL);
  pthread_mutex_init(&countTasks, NULL);
  
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

      pthread_cond_signal(&countCond);
      pthread_cond_wait(&taskCond, &q->lock);
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
      node->task(node->i); //Run task;

      pthread_mutex_lock(&countTasks);
      ++tasksFinished;
      pthread_mutex_unlock(&countTasks);
      
    }
  }
}

void addTask(void(*task)(int), int i){
  taskNode_t * node = (taskNode_t *)malloc(sizeof(taskNode_t));
  node->task = task;
  node->next = NULL;
  node->i = i;

  pthread_mutex_lock(&q->lock);

  if(q->tail != NULL){
    q->tail->next = node;
  }
  q->tail = node;

  if(q->head == NULL){
    q->head = node;
  }

  pthread_cond_signal(&taskCond);
  pthread_mutex_unlock(&q->lock);
}

#endif
