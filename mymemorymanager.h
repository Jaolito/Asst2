#ifndef MYMEMORYMANAGER_H
#define MYMEMORYMANAGER_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include "my_pthread_t.h"



/* Structs */

typedef struct free_frame {
	int page_frame;
	struct free_frame * next;
}*free_frame_ptr;

typedef struct page_meta {
	int page_frame;
	char in_pm;
	int tid;
	struct page_meta * next;
	mementryPtr head;
	mementryPtr middle;
}*page_meta_ptr;


typedef enum {THREADREQ, LIBRARYREQ} req_type;

/* Functions */

void * myallocate(unsigned int x, char * file, int line, req_type rt);

int mydeallocate(void * x, char * file, int line, req_type rt);

void page_enqueue(free_frame_ptr node);

free_frame_ptr page_dequeue();

void swap_handler(int signum);

void swap_pages(int src_frame, int dest_frame);

#endif
