all: my_pthread.o mymemorymanager.h
	gcc -Wall -o test mymemorymanager.c my_pthread.o -g

my_pthread.o: my_pthread.c my_pthread_t.h
	gcc -Wall -c my_pthread.c -g
	
mymemorymanager.o: mymemorymanager.h
	gcc -Wall -c mymemorymanager.c -g
	
test: mymemorymanager.o test.c
	gcc -Wall -o test test.c mymemorymanager.o -g

clean:
	rm *.o test
