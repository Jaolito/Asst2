#ifndef MYMEMORYMANAGER_H
#define MYMEMORYMANAGER_H

#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)

/* Structs */

typedef struct page_meta {
	int page_frame;
	struct page_meta * next;
}*page_meta_ptr;



typedef enum {THREADREQ, LIBRARYREQ} req_type;

/* Functions */

void * myallocate(unsigned int x, char * file, int line, req_type rt);

int mydeallocate(void * x, char * file, int line, req_type rt);

void page_enqueue(page_meta_ptr node);

page_meta_ptr page_dequeue();

#endif
