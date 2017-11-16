#include <stdlib.h>
#include <stdio.h>
#include "mymemorymanager.h"


typedef struct __myarg_t { 
	int a;
	int b; 
} myarg_t;

void * mythread2(void *arg) {
	
	myarg_t *m = (myarg_t *) arg;
	
	printf("pre-malloc\n");
	int * intArray = (int *) malloc(sizeof(int)*2);
	printf("post-malloc\n");
	printf("intArray thread2 address: %p\n", (void *)intArray);
	intArray[0] = m->a;
	intArray[1] = m->b;
	
	printf("[%d, %d]\n", intArray[0], intArray[1]);
	
	int * intArray2 = (int *) malloc(sizeof(int)*30);
	
	printf("intArray2 thread2 address: %p\n", (void *)intArray2);
	
	my_pthread_exit(arg);
	return NULL;
}

void *mythread(void *arg){
	
	myarg_t *m = (myarg_t *) arg;
	
	printf("pre-malloc\n");
	int * intArray = (int *) malloc(sizeof(int)*2);
	printf("post-malloc\n");
	printf("intArray address: %p\n", (void *)intArray);
	intArray[0] = m->a;
	intArray[1] = m->b;
	
	printf("[%d, %d]\n", intArray[0], intArray[1]);
	
	int * intArray2 = (int *) malloc(sizeof(int)*30);
	int i;
	for (i = 0; i < 20; i++) {
		intArray2[i] = i*100;
	}
	
	printf("intArray2 address: %p\n", (void *)intArray2);
	
	my_pthread_t thread;
	myarg_t arg2;
	arg2.a=30;
	arg2.b=50;
	my_pthread_create(&thread, NULL, mythread2, &arg2);
	my_pthread_join(thread, NULL);
	
	printf("intArray address: %p\n", (void *)intArray);
	
	printf("intArray2 address: %p\n", (void *)intArray2);
	
	printf("[%d, %d]\n", ((int *)(((void *)intArray) + 4096))[0], ((int *)(((void *)intArray) + 4096))[1]);
	
	my_pthread_exit(arg);
	return NULL;
}


int main() {
	
	my_pthread_t mp;
	myarg_t args1;
	args1.a = 10;
	args1.b = 20;
	my_pthread_create(&mp, NULL, mythread, &args1);
	my_pthread_join(mp, NULL);
		
	return 0;
}
