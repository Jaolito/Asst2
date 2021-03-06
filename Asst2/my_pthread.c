// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server: test

#include "my_pthread_t.h"
#include "mymemorymanager.h"

extern mementryPtr head;
extern mementryPtr middle;
static const int mycode = 1234567890;

pLevels * running_qs = NULL;
queue * waiting_queue = NULL;
extern context_node * current;
queue * join_queue = NULL;
exit_node * exit_list = NULL;


//Update Flags
flagCalled fc = FIRST;
int firstThread = 1;
unsigned int maintenanceCount = 0;
extern int mem_flag;

//Timer
struct itimerval itv;

uint threadCount = 1;
int mutex_count = 0;


/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	
	if (firstThread) {
		mem_flag = 0;
	}
	
	//printf("In pthread_create\n");
	void (*exit_function) (void *) = &my_pthread_exit;
	ucontext_t *exit_context = (ucontext_t *)myallocate((sizeof(ucontext_t)), NULL, 0, LIBRARYREQ);
	mem_flag = 0;
	getcontext(exit_context);
	exit_context -> uc_link=0;
 	exit_context -> uc_stack.ss_sp = myallocate(2080, NULL, 0, LIBRARYREQ);
 	mem_flag = 0;
 	exit_context -> uc_stack.ss_size=MEM;
 	exit_context -> uc_stack.ss_flags=0;
 	makecontext(exit_context, (void*)exit_function, 1, NULL);

	
	ucontext_t *new_context, *old_context;
	new_context = (ucontext_t *)myallocate((sizeof(ucontext_t)), NULL, 0, LIBRARYREQ);
	mem_flag = 0;
	getcontext(new_context);
	tcb * new_thread_block = (tcb *)myallocate((sizeof(tcb)), NULL, 0, LIBRARYREQ);
	mem_flag = 0;
	context_node * new_node = (context_node *)myallocate((sizeof(context_node)), NULL, 0, LIBRARYREQ);
	mem_flag = 0;
	
	if (firstThread) {
		firstThread = 0;
		
		createScheduler();
		
		old_context = (ucontext_t *)myallocate((sizeof(ucontext_t)), NULL, 0, LIBRARYREQ);
		mem_flag = 0;
		context_node * old_node = (context_node *)myallocate((sizeof(context_node)), NULL, 0, LIBRARYREQ);
		mem_flag = 0;
		tcb * old_thread_block = (tcb *)myallocate((sizeof(tcb)), NULL, 0, LIBRARYREQ);
		mem_flag = 0;
		getcontext(old_context);
		
		//Creates new thread context and new thread node 
		new_context -> uc_link=exit_context;
 		new_context -> uc_stack.ss_sp= myallocate(MEM, NULL, 0, LIBRARYREQ);
 		mem_flag = 0;
 		new_context -> uc_stack.ss_size=MEM;
 		new_context -> uc_stack.ss_flags=0;
 		makecontext(new_context, (void*)function, 1, arg);
		new_thread_block -> thread_context = new_context;
		new_thread_block -> tid = 1;
		new_thread_block -> thread_priority = 0;
		new_thread_block -> join_id = -1;
		new_thread_block -> value_ptr = NULL;
		new_thread_block -> mid = -1;
		new_thread_block -> first_page = NULL;
		new_thread_block -> malloc_frame = 0;
		
		new_node -> thread_block = new_thread_block;
		new_node -> next = NULL;
		
		enqueuee(new_node, running_qs -> rqs[0]);
		
		//Gets old thread context and creates old thread node 
		old_thread_block -> thread_context = old_context;
		old_thread_block -> tid = 0;
		old_thread_block -> thread_priority = 0;
		old_thread_block -> join_id = -1;
		old_thread_block -> value_ptr = NULL;
		old_thread_block -> mid = -1;
		old_thread_block -> first_page = NULL;
		old_thread_block -> malloc_frame = 0;
		
		old_node -> thread_block = old_thread_block;
		old_node -> next = NULL;
		
		current = old_node;
		
		enqueuee(old_node, running_qs -> rqs[0]);
		
		
		//Both context nodes are enqueueed to the running queue
		
	} else {
		//Creates new thread context and new thread node 
		new_context -> uc_link=exit_context;
 		new_context -> uc_stack.ss_sp= myallocate(MEM, NULL, 0, LIBRARYREQ);
 		mem_flag = 0;
 		new_context -> uc_stack.ss_size=MEM;
 		new_context -> uc_stack.ss_flags=0;
 		makecontext(new_context, (void*)function, 1, arg);
		new_thread_block -> thread_context = new_context;
		new_thread_block -> tid = threadCount;
		new_thread_block -> thread_priority = 0;
		new_thread_block -> join_id = -1;
		new_thread_block -> value_ptr = NULL;
		new_thread_block -> mid = -1;
		new_thread_block -> first_page = NULL;
		new_thread_block -> malloc_frame = 0;
		
		new_node -> thread_block = new_thread_block;
		new_node -> next = NULL;
		
		enqueuee(new_node, running_qs -> rqs[0]);
	}
	
	*thread = threadCount;
	
	threadCount++;
	
	scheduler();

	return 0;
}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	
	if (running_qs->rqs[current->thread_block->thread_priority]->front->next == NULL) {
		return 0;
	} else {
		fc = YIELD;
		scheduler();
	}
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	
	//printf("In pthread_exit\n");
	
	int i;
	exit_node * e = (exit_node *)myallocate((sizeof(exit_node)), NULL, 0, LIBRARYREQ);
	mem_flag = 0;
	e -> next = NULL;
	e -> tid = current->thread_block->tid;
	e -> value_ptr = value_ptr;
	context_node * temp = NULL;
	
	fc = PEXIT;
	
	for (i = 0; i < get_specific_count(join_queue); i++) {
		temp = dequeuee(join_queue);
		if (temp->thread_block->join_id == current->thread_block->tid) {
			temp->thread_block->value_ptr = value_ptr;
			temp->thread_block->thread_priority = 0;
			enqueuee(temp, running_qs -> rqs[0]);
		} else {
			enqueuee(temp, join_queue);
		}
	}
	
	if (exit_list == NULL) {
		exit_list = e;
	} else {
		e->next = exit_list;
		exit_list = e;
	}
	
	scheduler();
	
	return;
}

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	
	exit_node * temp = exit_list;
	
	if ((uint)thread > threadCount-1) {
		//Thread doesn't exist
		return -1;
	}
	
	while (temp != NULL) {
		if (temp->tid == thread) {
			if (value_ptr != NULL) {
				*value_ptr = temp->value_ptr;
			}
			return 0;
		} else {
			temp = temp -> next;
		}
	}
	
	current->thread_block->join_id = thread;
	
	fc = JOIN;
	
	scheduler();
	
	if (value_ptr != NULL) {
		*value_ptr = current->thread_block->value_ptr;
	}
	
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	
	//mutex = (my_pthread_mutex_t *) malloc(sizeof(my_pthread_mutex_t));
	
	mutex->mid = mutex_count++;
	mutex->locked = 0;
	mutex->tid = -1;
	mutex->ready_waiting = 0;
	
	
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	
	if (mutex == NULL) {
		return -1;
	}
	
	while (mutex->locked) {
		fc = BLOCKED;
		current->thread_block->mid = mutex->mid;
		scheduler();
	} 
	mutex->locked = 1;
	mutex->tid = current->thread_block->tid;
	mutex->ready_waiting = 0;
	
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	
	if (mutex == NULL) {
		return -1;
	}
	
	if (mutex->locked == 0) {
		//Can't unlock an unlocked mutex
		return -1;
	} else {
		if (mutex->tid != current->thread_block->tid) {
			//Wrong thread calling unlock
			return -1;
		} else {
			mutex->locked = 0;
			mutex->tid = -1;
			context_node * temp = waiting_queue->front;
			while (temp != NULL) {
				if (mutex->mid == temp->thread_block->mid) {
					temp->thread_block->mid = -1;
					temp->thread_block->thread_priority = 0;
					mutex->ready_waiting = 1;
					enqueuee(dequeuee(waiting_queue), running_qs->rqs[0]);
					return 0;
				}
				temp = temp->next;
			}
		}
	}
	
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	
	if (mutex == NULL) {
		return -1;
	}
	
	if (mutex->locked) {
		return -1;
	}
	
	if (mutex->ready_waiting) {
		return -1;
	}
	
	context_node * temp = waiting_queue->front;
	
	while (temp != NULL) {
		if (mutex->mid == temp->thread_block->mid) {
			return -1;
		}
		temp = temp->next;
	}
	//free(mutex);
	mutex = NULL;
	
	return 0;
};


/* ******************* SCHEDULER ******************* */


void createScheduler() {
	
	int i;
	running_qs = (pLevels *)myallocate((sizeof(pLevels)), NULL, 0, LIBRARYREQ);	
	mem_flag = 0;
	for (i = 0; i < NUM_PRIORITIES; i++) {
		running_qs->rqs[i] = (queue *)myallocate((sizeof(queue)), NULL, 0, LIBRARYREQ);
		mem_flag = 0;
		running_qs->rqs[i]->front = NULL;
		running_qs->rqs[i]->back = NULL;
	}
	waiting_queue = (queue *)myallocate((sizeof(queue)), NULL, 0, LIBRARYREQ);
	mem_flag = 0;
	waiting_queue->front = NULL;
	waiting_queue->back = NULL;
	
	join_queue = (queue *)myallocate((sizeof(queue)), NULL, 0, LIBRARYREQ);
	mem_flag = 0;
	join_queue->front = NULL;
	join_queue->back = NULL;
	
	//scheduler called when timer goes off
	signal(SIGALRM, timer_triggered);
}

void timer_triggered(int signum) {
	//printf("TIMER\n");
	fc = TIMER;
	scheduler();
}

void scheduler() {
	
	//printf("Scheduler running, mem_flag = %d\n", mem_flag);
	if (mem_flag) {
		int t = current->thread_block->thread_priority;
		itv.it_value.tv_usec = (25 + 25 * t) * 1000;
		itv.it_value.tv_sec = 0;
		itv.it_interval = itv.it_value;
		setitimer(ITIMER_REAL, &itv, NULL);
		//printf("Timer Reset\n");
		return;
	}
	
	if (updateQueue()) {
		//Let current thread resume
	} else {
		
		context_node * prnt = running_qs->rqs[0]->front;
		//printf("Scheduler: pqueue: ");
		while(prnt != NULL) {
			//printf("%d ", prnt->thread_block->tid);
			prnt = prnt->next;
		}
		//printf("\n");
		
		maintenanceCount++;
		int i;
		context_node * new_context;
		for (i = 0; i < NUM_PRIORITIES; i++) {
			new_context = running_qs -> rqs[i] -> front;
			//printf("New context found\n");
			if (new_context != NULL) {
				break;
			}
		}
		if (new_context == NULL) {
			return;
			//all queues empty, do something
		}
		
		if (current != NULL) {
		
			if (new_context->thread_block->tid == current->thread_block->tid) {
				//printf("Main twice \n");
				if (new_context->next != NULL) {
					new_context = new_context->next;
					//printf("Had next: tid is %d \n", new_context->thread_block->tid);
				} else {
					for (i = i+1; i < NUM_PRIORITIES; i++) {
						new_context = running_qs -> rqs[i] -> front;
						if (new_context != NULL) {
							break;
						}
					}
				}
				//printf("DqEq: %d\n", running_qs->rqs[i]->front->thread_block->tid);
				enqueuee(dequeuee(running_qs->rqs[current->thread_block->thread_priority]), running_qs->rqs[current->thread_block->thread_priority]);
				if (new_context == NULL) {
					new_context = current;
				}
			}
		}		
		int t = new_context->thread_block->thread_priority;
		itv.it_value.tv_usec = (25 + 25 * t) * 1000;
		itv.it_value.tv_sec = 0;
		itv.it_interval = itv.it_value;
		 
		if(setitimer(ITIMER_REAL, &itv, NULL) == -1){
			//print error
			return;
		}
		
		
		
		if (fc == PEXIT) {
			//printf("SET\n");
			fc = NONE;
			current = new_context;
			setcontext(new_context->thread_block->thread_context);
		} else {
			//printf("SWAP\n");
			context_node * old = current;
			current = new_context;
			lock_mem();
			//printf("Switching\n");
			swapcontext(old->thread_block->thread_context, new_context->thread_block->thread_context);
		}
		
	}
	
}

//Updates queues if necessary. Return 1 if current thread should resume
int updateQueue(){
	
	//Determine what to do with current thread
	switch(fc) {
		case NONE: 
			//printf("NONE CASE\n");
			return 1;
		case FIRST: 
			//printf("FIRST CASE\n");
			return 0;
		case TIMER: 
			printf("TIMER CASE\n");
			if (current->thread_block->thread_priority < NUM_PRIORITIES - 1) {
				current->thread_block->thread_priority++;
				enqueuee(dequeuee(running_qs->rqs[current->thread_block->thread_priority-1]), running_qs->rqs[current->thread_block->thread_priority]);
			} else {
				enqueuee(dequeuee(running_qs->rqs[current->thread_block->thread_priority]), running_qs->rqs[current->thread_block->thread_priority]);
			}
			fc = NONE;
			break;
		case YIELD: 
			//printf("YIELD CASE\n");
			enqueuee(dequeuee(running_qs->rqs[current->thread_block->thread_priority]), running_qs->rqs[current->thread_block->thread_priority]);
			fc = NONE;
			break;
		case PEXIT:
			//printf("PEXIT CASE\n");
			dequeuee(running_qs -> rqs[current->thread_block->thread_priority]); 
			if (current->thread_block->tid > 0) {
				//freeContext(current);
			}
			current = NULL;
			break;
		case JOIN: 
			//printf("JOIN CASE\n");
			enqueuee(dequeuee(running_qs->rqs[current->thread_block->thread_priority]), join_queue);
			fc = NONE;
			break;
		case BLOCKED:
			enqueuee(dequeuee(running_qs->rqs[current->thread_block->thread_priority]), waiting_queue);
			fc = NONE;
			break;
	}
	
	
	//move oldest (lowest priority) threads to the end of the highest priority queue
	if (maintenanceCount > MAINT_CYCLE) {
		
		maintenanceCount = 0;
		
		queue * highest = running_qs->rqs[0];
		queue * lowest = running_qs->rqs[NUM_PRIORITIES - 1];
		context_node * temp;
		
		if (lowest->front != NULL) {
			if (highest->front != NULL) {
				highest->back->next = lowest->front;
				highest->back = lowest->back;
				lowest->front = NULL;
				lowest->back = NULL;
			} else {
				highest->front = lowest->front;
				highest->back = lowest->back;
				lowest->front = NULL;
				lowest->back = NULL;
			}
			
			temp = highest->front;
			while (temp != NULL) {
				temp->thread_block->thread_priority = 0;
				temp = temp->next;
			}
		}
	}
	
	return 0;
}

/*void freeContext(context_node * freeable) {
	//free(freeable->thread_block->thread_context->uc_stack.ss_sp);
	free(freeable->thread_block->thread_context);
	free(freeable->thread_block);
	free(freeable);
}*/

/* Queues */

void enqueuee(context_node * enter_thread, queue * Q){
	if(Q->front == NULL){
		Q -> front = enter_thread;
		Q -> back = enter_thread;
		enter_thread -> next = NULL;
	} else {
		Q -> back -> next = enter_thread;
		Q -> back = enter_thread;
		Q -> back -> next = NULL;
	}
}

context_node * dequeuee(queue * Q){

	context_node * temp;
	if(Q->front == NULL){
		return NULL;
	} else {
		temp = Q -> front;
		Q -> front = temp -> next;
	}
	
	return temp;
}

int get_specific_count(queue * Q){
	
	int count = 0;
	context_node * temp = Q -> front;
	while(temp!=NULL){
		count++;
		temp = temp -> next;
	}
	return count;
	
}

void printAll() {
	int i;
	for (i = 0; i < NUM_PRIORITIES; i++) {
		context_node * temp = running_qs->rqs[i]->front;
		printf("PRIORITY %d : ", i);
		while (temp != NULL) {
			
			printf("%d ", temp->thread_block->tid);
			temp = temp->next;
			
		}
		printf("\n");
		
	}
}



void * mymalloc(unsigned int x, char * file, int line, void * memptr, int size) {
	
	
	/*
	 * If nothing has been allocated yet, this creates and initializes the first mementry struct which is pointed to by head and middle.
	 */
	if ( head == NULL) {
		head = (mementryPtr)memptr;
		head->size = size - sizeof(struct mementry);
		head->code = mycode;
		head->free = 1;
		head->next = NULL;
		head->prev = NULL;
		middle = head;
	}
	
	mementryPtr walker = head;		//Creates a pointer to traverse the mementry list.
	if(x < 100) {					//If the size requested is below 100 bytes, we will allocate the memory from the left.
		while(walker != NULL) {		//Traverses the mementry list until it finds a free spot. If not found, memory cannot be allocated.
			
			if(walker->free == 1) {
				if(walker->size >= x) {
					if(walker->size > (x + sizeof(struct mementry))) {		//If there's space to create a new mementry as well as the requested space...
						mementryPtr temp = (mementryPtr)(((char *)walker) + sizeof(struct mementry) + x);	//Creates a new mementry after the allocated space.
						
						temp->code = mycode;								//Initializes the values for the new mementry. Re-assigns pointers.
						temp->size = walker->size - sizeof(struct mementry) - x;
						temp->free = 1;
						temp->next = walker->next;
						temp->prev = walker;
						if(walker->next != NULL) {
							walker->next->prev = temp;
						}
						
						walker->size = x;									//Change values for the old pointer.
						walker->free = 0;
						walker->next = temp;
						
						if(walker == middle){
							middle = temp;
						}
						
						//current->thread_block->head = head;
						//current->thread_block->middle = middle;
						return walker+1;					//Returns the pointer to the requested space.
					}else {
						walker->free = 0;
						
						//current->thread_block->head = head;
						//current->thread_block->middle = middle;
						return walker+1;					//Returns the pointer to the requested space.
					}
				}else {
					walker = walker->next;
				}
			}else {
				walker = walker->next;
			}
		}
	}else {								//Larger requests will be allocated from the right. 
		walker = middle->next;
		while(walker != NULL){			//Checks for free space to the right of the middle.
			if(walker->free == 1) {
				if(walker->size >= x) {
					if(walker->size > (x + sizeof(struct mementry))) {			//If there's space to create a new mementry as well as the requested space...
						mementryPtr temp = (mementryPtr)(((char *)walker) + sizeof(struct mementry) + x);	//Creates a new mementry after the allocated space.
						
						temp->code = mycode;												//Initializes the values for the new mementry. Re-assigns pointers.
						temp->size = walker->size - sizeof(struct mementry) - x;
						temp->free = 1;
						temp->next = walker->next;
						temp->prev = walker;
						if(walker->next != NULL) {
							walker->next->prev = temp;
						}
						
						walker->size = x;													//Change values for the old pointer.
						walker->free = 0;
						walker->next = temp;
						
						//current->thread_block->head = head;
						//current->thread_block->middle = middle;
						return walker+1;					//Returns the pointer to the requested space.
					}else {
						walker->free = 0;
						
						//current->thread_block->head = head;
						//current->thread_block->middle = middle;
						return walker+1;					//Returns the pointer to the requested space.
					}
				}else {
					walker = walker->next;
				}
			}else {
				walker = walker->next;
			}
		}
		walker = middle;
		while(walker != NULL) {				//Traverses left from the middle looking for free space.
			if(walker->free == 1) {
				if(walker->size >= x) {
					if(walker->size > (x + sizeof(struct mementry))) {			//If there's space to create a new mementry as well as the requested space...
						mementryPtr temp = (mementryPtr)((char *)walker + sizeof(struct mementry) + (walker->size - x - sizeof(struct mementry)));
						
						temp->code = mycode;												//Initializes the values for the new mementry. Re-assigns pointers.
						temp->size = x;
						temp->free = 0;
						temp->next = walker->next;
						temp->prev = walker;
						if(walker->next != NULL) {
							walker->next->prev = temp;
						}
						
						walker->size = walker->size - x - sizeof(struct mementry);			//Change values for the old pointer.
						walker->free = 1;
						walker->next = temp;
						
						//current->thread_block->head = head;
						//current->thread_block->middle = middle;
						return temp+1;					//Returns the pointer to the requested space.
					}else {
						walker->free = 0;
						
						//current->thread_block->head = head;
						//current->thread_block->middle = middle;
						return walker+1;					//Returns the pointer to the requested space.
					}
				}else {
					walker = walker->prev;
				}
			}else {
				walker = walker->prev;
			}
		}
	}
	//If you reach here, the space could not be allocated.
	//printf("mymalloc could not allocate the requested space at line %d of file %s.\n", line, file);
	return NULL;
}

void myfree(void * x, char * file, int line) {
	
	if( (char *)x < (char *)(head) + sizeof(struct mementry) || (char *)x > (char *)(head) + 4999) {
		printf("Pointer given by line %d of file %s is out of bounds.\n", line, file);
		return;
	}
	mementryPtr freeer = (mementryPtr)((char *)x - sizeof(struct mementry));
	if(freeer->code == mycode) {
		if(freeer->free == 1) {
			printf("Double Free attempted at line %d of file %s.\n", line, file);
			return;
		}else {
			freeer->free = 1;
		}
	}else {
		printf("Attempted to free memory not allocated by mymalloc() at line %d of file %s.\n", line, file);
		return;
	}
	
	
	/*
	 * If the previous mementry struct was free, the two are combined.
	 */
	if(freeer->prev != NULL && freeer->prev->free == 1) {
		if(freeer->next != NULL) {
			freeer->next->prev = freeer->prev;
		}
		if(freeer->prev != NULL) {
			freeer->prev->next = freeer->next;
		}
		freeer->prev->size += sizeof(struct mementry) + freeer->size;
		if(freeer == middle) {
			middle = freeer->prev;
		}
		freeer = freeer->prev;
	}
	/*
	 * If the next mementry struct was free, the two are combined.
	 */
	if(freeer->next != NULL && freeer->next->free == 1) {
		mementryPtr temp = freeer->next;
		if(temp->next != NULL) {
			temp->next->prev = temp->prev;
		}
		if(temp->prev != NULL) {
			temp->prev->next = temp->next;
		}
		freeer->size += sizeof(struct mementry) + temp->size;
		if(temp == middle) {
			middle = freeer;
		}
	}
	return;
}

int free_mem_count() {
	mementryPtr walker = head;
	int rtn = 0;
	
	while(walker != NULL) {
		if (walker->free) {
			rtn += walker->size;
		}
		walker = walker->next;
	}
	
	return rtn;
}
