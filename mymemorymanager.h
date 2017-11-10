#ifndef MYMEMORYMANAGER_H
#define MYMEMORYMANAGER_H

#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)

/* Structs */

typedef struct page_meta {
	char free;
	my_pthread_t tid;
}*page_meta_ptr;


typedef enum {THREADREQ, LIBRARYREQ} req_type;

/* Functions */

void * myallocate(unsigned int x, char * file, int line, req_type rt);

int mydeallocate(void * x, char * file, int line, req_type rt);

#endif
