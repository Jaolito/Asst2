#ifndef MYMEMORYMANAGER_H
#define MYMEMORYMANAGER_H




#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include "my_pthread_t.h"

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)

/* Structs */



typedef struct page_meta {
	int page_frame;
	int tid;
	int num_pages;
	int free_page_mem;
	struct page_meta * next;
	mementryPtr head;
	mementryPtr middle;
}*page_meta_ptr;


typedef enum {THREADREQ, LIBRARYREQ} req_type;

/* Functions */

void * myallocate(unsigned int x, char * file, int line, req_type rt);

int mydeallocate(void * x, char * file, int line, req_type rt);

void swap_handler(int signum);

void swap_pages(int src_frame, int dest_frame);

#endif
