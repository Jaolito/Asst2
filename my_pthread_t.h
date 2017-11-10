// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server: 
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

#define NUM_PRIORITIES 100
#define MAINT_CYCLE 200
#define MEM 64000

#define pthread_ my_pthread_
#define USE_MY_PTHREAD 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>



/* define your data structures here: */

// Feel free to add your own auxiliary data structures

typedef uint my_pthread_t;

typedef struct threadControlBlock {
	my_pthread_t tid;
	ucontext_t * thread_context;
	unsigned int thread_priority;
	my_pthread_t join_id;
	void * value_ptr;
	int mid;
} tcb; 

typedef struct context_node { 
	tcb * thread_block;
	struct context_node * next;
} context_node;

typedef struct queue {
	/* Pointer to the front of the running_queue */
	context_node * front;
	context_node * back;
} queue;

//Priority levels for running queues
typedef struct pLevels {
	queue * rqs[NUM_PRIORITIES];
} pLevels;

//Exit nodes
typedef struct exit_node {
	my_pthread_t tid;
	void * value_ptr;
	struct exit_node * next;
} exit_node;


//Flags used to determine why the scheduler was called
typedef enum {NONE, TIMER, YIELD, PEXIT, JOIN, FIRST, BLOCKED} flagCalled;


/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */
	int mid;
	int locked;
	my_pthread_t tid;
	int ready_waiting;
} my_pthread_mutex_t;



/* Function Declarations: */

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

/* Helper Methods */

void timer_triggered(int signum);

void scheduler();

void createScheduler();

int updateQueue();

void freeContext(context_node * freeable);

context_node * dequeuee(queue * Q);

void enqueuee(context_node * enter_thread, queue * Q);

void printAll();

#ifdef USE_MY_PTHREAD
#define pthread_t my_pthread_t
#define pthread_mutex_t my_pthread_mutex_t
#define pthread_create my_pthread_create
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
#endif

#endif
