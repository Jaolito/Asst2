#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

static mementryPtr head, middle;
static char myblock[5000];
static const int mycode = 1234567890;

void * mymalloc(unsigned int x, char * file, int line) {
	
	
	/*
	 * If nothing has been allocated yet, this creates and initializes the first mementry struct which is pointed to by head and middle.
	 */
	if ( head == NULL) {
		head = (mementryPtr)myblock;
		head->size = 5000 - sizeof(struct mementry);
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
						
						return walker+1;					//Returns the pointer to the requested space.
					}else {
						walker->free = 0;
						
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
						
						return walker+1;					//Returns the pointer to the requested space.
					}else {
						walker->free = 0;
						
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
						
						return temp+1;					//Returns the pointer to the requested space.
					}else {
						walker->free = 0;
						
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
	printf("mymalloc could not allocate the requested space at line %d of file %s.\n", line, file);
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
