# Asst2

Design 

Our 8MB of physical memory is broken down into 3 segments of memory blocks for allocated memory, shared allocated memory and OS data. There are 1596 pages available for allocation to various threads upon calling malloc(), 4 pages available for allocation upon calling shalloc() and a total of 33 threads that can be created and given a thread control block within the OS region. The number of total pages that can be allocated is determined by the total number of threads that can be handled by our memory manager and the individual stack size of each thread. Since each threads control block, which contains metadata for each thread, as well as the u_context for that thread, is located in the OS region of our physical memory, the more threads, or the larger the memory given to each thread's stack, the smaller the total number of pages available for allocation. 

*** mymalloc() function returns memory segments to the caller from within an actual page ***

Maintains a pointer to the head of the page and the middle of the page. Each mymalloc() call updates the specific head and middle for the current thread in order to resume the correct position for subsequent malloc calls. Large blocks of memory are given from the back (higher memory addresses) of the page and smaller blocks are given from the front. 

void * myallocate(unsigned int x, char * file, int line, req_type rt);

When malloc() is called, a macro replaces the malloc call with our method signature for myallocate(). If the thread library calls for the creation of a thread, or our own memory manager calls for the creation of page meta data, the allocation is handled by returning a pointer to the beginning of our OS region and incrementing the previous handle by the requested size in bytes. If the call is from a thread, the very first call to malloc initializes all of the neccessary memory for our manager to handle subsequent calls. 

Depending on the memory request size and the number of pages the thread has already been allocated is how our manager will handle the call. If the thread has not been given any pages yet, the manager evicts the current page held in the frame of our physical memory at index 0 and creates a page for the thread there, creating and updating the necessary meta data for that page. If the thread has already been given a page, our manager first attempts to allocate the requested size within within the pages that it has already given to the thread. Depending on whether or not there is enough space within the already allocated region the manager will then give another page to the thread. 

Every time a page is given to a thread, the specific index in physical memory is evicted and the allocation for that threads memory is directly correlated to the number of pages that it already has been given. A single thread can only be given 1596 pages, anything more than that will result in the return of NULL to the thread on request. 

SWAP SPACE:

Since every thread is returned a pointer starting from the 0 index frame of physical memory and as more pages are given, the pointers increment to the end of the allocation block, when threads are swapped in and out of execution because of the scheduler, their specific pages must be swapped in and out as well. 
void swap_handler(int sig, siginfo_t *si, void *unused);

To ensure that no thread can access memory pages that it was not directly allocated, the entire block of dynamic memory is protected from read and writes unless the manager is allocating memory or swapping pages. On the event that a page in our memory block is accessed, a segmentation fault signal is raised and caught by swap_handler(). We then use the signal to determine which one of the thread's pages was accessed, if that page is already in physical memory, and if not, where in swap memory is it positioned. The location of the thread's page is determined via that threads page_meta_data. Each page_meta maintains a pointer to the position of the next page. Once the location is calculated, the page currently in the frame on physical memory is swapped out and the new page that is ready to be read from, or written to properly is swapped in and that specific page frame is unprotected.

void * shalloc(size_t size);

A block of 4 pages is saved for shared allocation and a handle to the top of that block is used to allocate from it. A call to shalloc() updates the head and middle, calls mymalloc(), which returns an address and updates the head and middle after the allocation. 

Metadata 

The metadata is stored in the thread control block.  The metadata stores the page frame, thread id, next page, number of pages, free pages, head and middle.  We use this data to decide whether or not to swap pages. If the current page isn't in physical memory, we swap.  We also use this data to keep track of the current information of each page  and update it when necessary.

We completed phases A,B,C and D, and tested the program on interpreter.cs.rutgers.edu.  