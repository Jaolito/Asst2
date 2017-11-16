#include "mymemorymanager.h"
#include "my_pthread_t.h"
#include "my_pthread.c"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

void * myblock_ptr;
page_meta_ptr * frame_table;
char first_call = 1;
context_node * current;
mementryPtr head;
mementryPtr middle;
void * os_mem_ptr;
int swap_fd;
off_t swap_file_offset;
int page_counter = 0;

void * myallocate(unsigned int x, char * file, int line, req_type rt) {
	
	void * rtn_ptr;
	page_meta_ptr current_page_meta;
	//Change all uses of temp to ^


		
	if (first_call) {
		first_call = 0;
		signal(SIGSEGV, swap_handler);
		
		//May need to free later
		myblock_ptr = memalign( sysconf(_SC_PAGESIZE), 8388608);
		printf("myblock_ptr address: %p\n", myblock_ptr);
		//Swap space
		swap_fd = open("swap_file",O_RDWR);
		
		//myswap_ptr = memalign( sysconf(_SC_PAGESIZE), 8388608*2);
		//6015 pages total
		os_mem_ptr = myblock_ptr + 7860224;
		/*
		*
		*  8118272- (16677216/4096)*40  = (4101 + sizeof(page_meta)) num_frames
		*
		*	40 is size of page_meta
		*/
		frame_table = (page_meta_ptr *)myallocate(sizeof(page_meta_ptr)*1919, NULL, 0, LIBRARYREQ);
	}
	
	//If the current thread has a page
	if(rt == LIBRARYREQ){
		printf("Library Call\n");
		void * temp = os_mem_ptr;
		os_mem_ptr += x;
		return temp;
	}
	if (current->thread_block->first_page != NULL) {
		head = current->thread_block->head;
		middle = current->thread_block->middle;
	} else { //Current thread does not have its first page yet
		
		//Total number of potential pages not exceeded
		if (page_counter < 6015) {
			swap_pages(0, -1);
			current_page_meta = (page_meta_ptr)myallocate(sizeof(struct page_meta), NULL, 0, LIBRARYREQ);
			current_page_meta->page_frame = 0;
			current_page_meta->tid = current->thread_block->tid;
			current_page_meta->next = NULL;
			current_page_meta->head = NULL;
			current_page_meta->middle = NULL;
			current->thread_block->first_page = current_page_meta;
			frame_table[0] = current_page_meta;
		} else {//No free pages in physical memory, check swap file
			//check if swapfile is full
		}
		head = NULL;
		middle = NULL;
	}
	
	rtn_ptr = mymalloc(x, file, line, myblock_ptr + current->thread_block->malloc_frame*4096, 4096);
	current_page_meta->head = head;
	current_page_meta->middle = middle;
	
	//If there was no space in the page for the request...
	if (rtn_ptr == NULL) {
		//allocate new page from free list
		if (page_counter < 6015) {
			current->thread_block->malloc_frame++;
			
			//Swaps the page after the current page to free space for contiguous pages
			swap_pages(current->thread_block->malloc_frame, -1);
			current_page_meta = (page_meta_ptr)myallocate(sizeof(struct page_meta), NULL, 0, LIBRARYREQ);
			current_page_meta->page_frame = current->thread_block->malloc_frame;
			current_page_meta->tid = current->thread_block->tid;
			current_page_meta->next = NULL;
			current_page_meta->head = NULL;
			current_page_meta->middle = NULL;
			frame_table[current->thread_block->malloc_frame] = current_page_meta;
			frame_table[current->thread_block->malloc_frame-1]->next = current_page_meta;
		} else {
			//check if swapfile is full
		}
		head = NULL;
		middle = NULL;
		
		rtn_ptr = mymalloc(x, file, line, myblock_ptr + current->thread_block->malloc_frame*4096, 4096);
		current_page_meta->head = head;
		current_page_meta->middle = middle;
	}
	return rtn_ptr;
}

int mydeallocate(void * x, char * file, int line, req_type rt){
	
	head = current->thread_block->head;
	middle = current->thread_block->middle;
	
	//Check to see if proper page is in frame. Swap if not
	int mem_offset = (int)(x - myblock_ptr);
	int index = mem_offset / 4096;
	int i;
	
	if (frame_table[index]->tid == current->thread_block->tid) {
		//Check myfree return value for errors
		head = frame_table[index]->head;
		middle = frame_table[index]->middle;
		myfree(x, file, line);
	} else {
		page_meta_ptr temp = current->thread_block->first_page;
		
		for (i = 0; i < index; i++) {
			temp = temp->next;
		}
		
		//Assuming temp is in physical memory
		swap_pages(index, temp->page_frame);
		head = temp->head;
		middle = temp->middle;
		myfree(x, file, line);
	}
	
	//How do we tell a thread that one of its pages has free space if it has multiple pages
	
	return 0;
}
/*
int main(){
	
	unsigned int sz = sysconf(_SC_PAGESIZE);
	printf("%d\n", sz);
	
	printf("%d\n", (int)sizeof(struct mementry));
	printf("%d\n", (int)sizeof(struct page_meta));
	
	return 0;
}*/

/* Helper Functions */

void swap_handler(int signum) {
	
	/*
	 * Check page table to see if the page being accessed belongs to the current thread.
	 * 		if so, change that page's mprotect to allow read/write: mprotect( myblock_ptr + frame_number * 4096, 4096, PROT_READ | PROT_WRITE);
	 * 		else, find the page they want and swap pages
	 */
	
}

void swap_pages(int src_frame, int dest_frame) {

	void * temp[4096];

	int swap_index;
	
	if (dest_frame == -1) {

		if(page_counter < 1919){
			//Swap to new frame from free_queue_head
			memcpy(myblock_ptr + page_counter*4096, myblock_ptr + src_frame * 4096, 4096);
			
			//Update page table
			if (frame_table[src_frame] != NULL) {
				frame_table[src_frame]->page_frame = page_counter;
				frame_table[page_counter] = frame_table[src_frame];
				frame_table[src_frame] = NULL;
			}
		}else{
			frame_table[src_frame]->page_frame = page_counter;
			swap_index = page_counter - 1919;
			swap_file_offset = lseek(swap_fd,swap_index*4096,SEEK_SET);
			write(swap_fd, myblock_ptr + src_frame * 4096, 4096);
			lseek(swap_fd, -swap_file_offset + 4096, SEEK_CUR);

		}
			page_counter++;
	}else {
		
		memcpy(temp, myblock_ptr + src_frame * 4096, 4096);
		memcpy(myblock_ptr + src_frame * 4096, myblock_ptr + dest_frame * 4096, 4096);
		memcpy(myblock_ptr + dest_frame * 4096, temp, 4096);
		
		//Update page table
		page_meta_ptr temp_pm = frame_table[src_frame];
		frame_table[src_frame] = frame_table[dest_frame];
		frame_table[dest_frame] = temp_pm;
		frame_table[src_frame]->page_frame = src_frame;
		frame_table[dest_frame]->page_frame = dest_frame;
	}
	
}



/***
*
* mprotect() to trigger SIGSEV, from the starting point until 1919*4096, not including "OS" region of physical memory
*
* lseek()  --- zero out 16MG of space???
*
* handle signal and swap correspondingly
*
* handle large allocations of multiple pages, TRICKY
*
* deallocation NOTE: keep track of how much free memory is in the block, member is entered in struct 
*
* document and comment code
*
* EXTRA: free space in physical memory 
*
***/
















