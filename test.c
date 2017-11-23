#include <stdlib.h>
#include <stdio.h>
#include "mymemorymanager.h"

int * arr1;

typedef struct __myarg_t { 
	int a;
	int b; 
} myarg_t;

void * mythread2(void *arg) {
	
	myarg_t *m = (myarg_t *) arg;
	
	int * intArray = (int *) malloc(sizeof(int)*2);
	//printf("intArray thread2 address: %p\n", (void *)intArray);
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
	printf("test: %d\n", intArray[0]);
	
	printf("intArray2 address: %p\n", (void *)intArray2);
	
	printf("[%d, %d]\n", intArray[0], intArray[1]);
	
	my_pthread_exit(arg);
	return NULL;
}

void *mythread4(void *arg){
	
	int * arr = (int *) malloc(4096*1596 - 32);
	int i;
	int count = 0;
	myarg_t *m = (myarg_t *) arg;
	int a = m->a;
	
	for (i = 0; i < 1634296; i++) {
		arr[i] = 1;
		if (i % 2000 == 0) {
			//printf("Yield4\n");
			//printf("Before Yield, i = %d\n", i);
			my_pthread_yield();
			//printf("After Yield, i = %d\n", i);
		}
	}
	
	for (i = 0; i < 1634296; i++) {
		count += arr[i];
	}

	printf("%d || %d\n", a, count);
	return NULL;
}

void *mythread6(void *arg){
	
	int * arr = (int *) malloc(4096*1596 - 32);
	int i;
	int count = 0;
	myarg_t *m = (myarg_t *) arg;
	int a = m->a;
	
	for (i = 0; i < 1634296; i++) {
		arr[i] = 2;
		if (i % 2000 == 0) {
			//printf("Yield6\n");
			//printf("Before Yield, i = %d\n", i);
			my_pthread_yield();
			//printf("After Yield, i = %d\n", i);
		}
	}
	
	for (i = 0; i < 1634296; i++) {
		count += arr[i];
	//	printf("%d | ", arr[i]);
	}

	printf("%d || %d\n", a, count);
	return NULL;
}

void *mythread3(void *arg){
	
	int * arr = (int *) malloc(4096 - 32);
	int i;
	int count = 0;
	myarg_t *m = (myarg_t *) arg;
	int a = m->a;
	
	for (i = 0; i < 1016; i++) {
		arr[i] = 1;
	}
	
	for (i = 0; i < 1016; i++) {
		count += arr[i];
	}
	printf("%d || %d\n", a, count);
	return NULL;
}

void *mythread9(void *arg){
	
	int i;
	int count = 0;
	myarg_t *m = (myarg_t *) arg;
	
	int * arr[4096];
	
	for (i = 0; i < 4096; i++) {
		arr[i] = (int *)malloc(4);
		arr[i][0] = m->a;
	}
	
	for (i = 0; i < 4096; i++) {
		count += arr[i][0];
	}
	
	printf("count = %d\n", count);
	
	printf("9 done\n");
	
	return NULL;
}

void *mythread5(void *arg){
	my_pthread_t mp;
	myarg_t args0;
	args0.a = 1;
	my_pthread_create(&mp, NULL, mythread4, &args0);
	
	my_pthread_t mp1;
	myarg_t args1;
	args1.a = 2;
	my_pthread_create(&mp1, NULL, mythread6, &args1);
	
	/*
	my_pthread_t mp2;
	myarg_t args2;
	args2.a = 3;
	my_pthread_create(&mp2, NULL, mythread9, &args2);
	my_pthread_join(mp2, NULL);
	
	my_pthread_t mp3;
	myarg_t args3;
	args3.a = 4;
	my_pthread_create(&mp3, NULL, mythread9, &args3);
	my_pthread_join(mp3, NULL);
	
	my_pthread_t mp4;
	myarg_t args4;
	args4.a = 5;
	my_pthread_create(&mp4, NULL, mythread9, &args4);
	args4.a = 6;
	my_pthread_create(&mp4, NULL, mythread9, &args4);
	args4.a = 7;
	my_pthread_create(&mp4, NULL, mythread9, &args4);
	args4.a = 8;
	my_pthread_create(&mp4, NULL, mythread9, &args4);
	args4.a = 9;
	my_pthread_create(&mp4, NULL, mythread9, &args4);
	args4.a = 10;
	my_pthread_create(&mp4, NULL, mythread9, &args4);
	
	*/
	printf("All Threads Created\n");
	
	my_pthread_join(mp, NULL);
	my_pthread_join(mp1, NULL);
	//my_pthread_join(mp4, NULL);
	return NULL;
}

void *mythread7(void *arg){
	
	printf("Before Shalloc\n");
	int * arr = (int *)shalloc(4096*2 - 32);
	printf("After Shalloc, %p\n", (void *)arr);
	int i;
	
	for (i = 0; i < 2040; i++) {
		arr[i] = 1;
	}
	printf("After initializing\n");
	
	arr1 = arr;
	
	//my_pthread_exit(NULL);
	return NULL;
}

void *mythread8(void *arg){
	
	int count;
	int i;
	printf("Before Accessing\n");
	for (i = 0; i < 2040; i++) {
		count += arr1[i];
		arr1[i] = 2;
	}
	
	printf("count = %d\n", count);
	
	return NULL;
}

void *mythread10(void *arg){
	
	void * arr = malloc(2500);
	printf("%p\n",arr);
	free(arr);
	arr = malloc(3000);
	printf("%p\n",arr);
	
	return NULL;
}



int main() {
	
	my_pthread_t mp;
	myarg_t args1;
	args1.a = 100000;
	my_pthread_create(&mp, NULL, mythread5, &args1);
	my_pthread_join(mp, NULL);
	
	printf("DONE\n");
	
	
		
	return 0;
}
