#ifndef MYMALLOC_H
#define MYMALLOC_H

#include <stdlib.h>

#define malloc( x ) mymalloc( (x), __FILE__, __LINE__ )
#define free( x ) myfree( (x), __FILE__, __LINE__ )

struct mementry {
	int size;
	int code;
	int free;
	struct mementry * next;
	struct mementry * prev;
};
typedef struct mementry * mementryPtr;

void * mymalloc(unsigned int x, char * file, int line, void * memptr, int size);
void myfree(void * x, char * file, int line);

#endif
