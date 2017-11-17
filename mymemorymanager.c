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
page_meta_ptr current_page_meta;

void * myallocate(unsigned int x, char * file, int line, req_type rt) {
	
	void * rtn_ptr;
	page_meta_ptr new_page_meta;

		
	if (first_call) {
		first_call = 0;
		signal(SIGSEGV, swap_handler);
		
		//May need to free later
		myblock_ptr = memalign( sysconf(_SC_PAGESIZE), 8388608);
		printf("myblock_ptr address: %p\n", myblock_ptr);
		//Swap space
		swap_fd = open("swap_file",O_RDWR | O_CREAT);
		lseek(swap_fd, 8338608*2, SEEK_SET);
		
		//myswap_ptr = memalign( sysconf(_SC_PAGESIZE), 8388608*2);
		//6004 pages total, 1908 frames in physical memory
		os_mem_ptr = myblock_ptr + 7815168;
		/*
		*
		*  8118272- (16677216/4096)*sizeof(page_meta)  = (4104 + sizeof(page_meta)) num_frames
		*
		*	40 is size of page_meta
		*/
		frame_table = (page_meta_ptr *)myallocate(sizeof(page_meta_ptr)*1908, NULL, 0, LIBRARYREQ);
	}
	
	//If the current thread has a page
	if(rt == LIBRARYREQ){
		//printf("Library Call\n");
		void * temp = os_mem_ptr;
		os_mem_ptr += x;
		return temp;
	}
	
	//If the request is greater than a page size...
	if (x > 4064) {
		
		//Calculate how many pages are needed for the request.
		int i;
		int num_of_pages = (x + 32) / 4096;
		
		if ((x+32) % 4096 > 0) {
			num_of_pages++;
		}
		
		//If first page, evict pages from the front.
		if (current->thread_block->first_page == NULL) {
			for (i = 0; i < num_of_pages; i++) {
				swap_pages(i, -1);
			}
			
			//Create page meta for the first of the big chunk
			page_meta_ptr front_meta_ptr = (page_meta_ptr)myallocate(sizeof(struct page_meta), NULL, 0, LIBRARYREQ);
			front_meta_ptr->page_frame = 0;
			front_meta_ptr->tid = current->thread_block->tid;
			front_meta_ptr->next = NULL;
			front_meta_ptr->head = NULL;
			front_meta_ptr->middle = NULL;
			front_meta_ptr->front_meta = front_meta_ptr;
			front_meta_ptr->num_pages = num_of_pages;
			front_meta_ptr->free_page_mem = -1;
			current->thread_block->first_page = front_meta_ptr;
			frame_table[0] = front_meta_ptr;
			
			//Create page metas for the rest of the pages of the big chunk
			for (i = 1; i < num_of_pages; i++) {
				new_page_meta = (page_meta_ptr)myallocate(sizeof(struct page_meta), NULL, 0, LIBRARYREQ);
				new_page_meta->page_frame = i;
				new_page_meta->tid = current->thread_block->tid;
				new_page_meta->next = NULL;
				frame_table[i-1]->next = new_page_meta;
				new_page_meta->head = NULL;
				new_page_meta->middle = NULL;
				new_page_meta->front_meta = front_meta_ptr;
				new_page_meta->num_pages = num_of_pages;
				new_page_meta->free_page_mem = -1;
				frame_table[i] = new_page_meta;
			}
			//Current thread block points to the last frame of the pages
			current->thread_block->malloc_frame = num_of_pages - 1;
			
			current_page_meta = new_page_meta;
		//If not first page, evict pages after the last frame used.
		} else {
			current->thread_block->malloc_frame++;
			for (i = current->thread_block->malloc_frame; i < current->thread_block->malloc_frame + num_of_pages; i++) {
				swap_pages(i, -1);
			}
			
			//Create page meta for the first of the big chunk
			page_meta_ptr front_meta_ptr = (page_meta_ptr)myallocate(sizeof(struct page_meta), NULL, 0, LIBRARYREQ);
			front_meta_ptr->page_frame = current->thread_block->malloc_frame;
			front_meta_ptr->tid = current->thread_block->tid;
			front_meta_ptr->next = NULL;
			frame_table[current->thread_block->malloc_frame-1]->next = front_meta_ptr;
			front_meta_ptr->head = NULL;
			front_meta_ptr->middle = NULL;
			front_meta_ptr->front_meta = front_meta_ptr;
			front_meta_ptr->num_pages = num_of_pages;
			front_meta_ptr->free_page_mem = -1;
			current->thread_block->first_page = front_meta_ptr;
			frame_table[front_meta_ptr->page_frame] = front_meta_ptr;
			
			//Create page metas for the rest of the pages of the big chunk
			for (i = front_meta_ptr->page_frame + 1; i < front_meta_ptr->page_frame + num_of_pages; i++) {
				current->thread_block->malloc_frame++;
				new_page_meta = (page_meta_ptr)myallocate(sizeof(struct page_meta), NULL, 0, LIBRARYREQ);
				new_page_meta->page_frame = i;
				new_page_meta->tid = current->thread_block->tid;
				new_page_meta->next = NULL;
				frame_table[i-1]->next = new_page_meta;
				new_page_meta->head = NULL;
				new_page_meta->middle = NULL;
				new_page_meta->front_meta = front_meta_ptr;
				new_page_meta->num_pages = num_of_pages;
				new_page_meta->free_page_mem = -1;
				frame_table[i] = new_page_meta;
			}
			
			current_page_meta = new_page_meta;
		}
		
		rtn_ptr = mymalloc(x, file, line, myblock_ptr + current_page_meta->front_meta->page_frame*4096, 4096*num_of_pages);
		
		//Head, Middle, and free memory are stored on the last page of the big chunk.
		current_page_meta->head = head;
		current_page_meta->middle = middle;
		current_page_meta->free_page_mem = free_mem_count();
		return rtn_ptr;
	}
	
	
	if (current->thread_block->first_page != NULL) {
		page_meta_ptr temp_ptr = frame_table[current->thread_block->malloc_frame];
		
		//If the page is not in frame, swap it in
		if (temp_ptr->tid != current->thread_block->tid) {
			//SWAP IN THE RIGHT PAGE
			temp_ptr = current->thread_block->first_page;
			while (temp_ptr->next != NULL) {
				temp_ptr = temp_ptr->next;
			}
			swap_pages(temp_ptr->page_frame, current->thread_block->malloc_frame);
		}
		current_page_meta = temp_ptr;
		
		//If the page is a part of a big chunk
		if (current_page_meta->front_meta != NULL) {
			int i;
			page_meta_ptr swap_ptr;
			
			swap_ptr = current_page_meta->front_meta;
			for (i = current_page_meta->front_meta->page_frame; i < current_page_meta->page_frame; i++) {
				temp_ptr = frame_table[i];
				if (temp_ptr != swap_ptr) {
					swap_pages(swap_ptr->page_frame, temp_ptr->page_frame);
				}
				swap_ptr = swap_ptr->next;
			}
		}
		head = current_page_meta->head;
		middle = current_page_meta->middle;
			 
	} else { //Current thread does not have its first page yet
		
		//Total number of potential pages not exceeded
		if (page_counter < 6015) {
			swap_pages(0, -1);
			new_page_meta = (page_meta_ptr)myallocate(sizeof(struct page_meta), NULL, 0, LIBRARYREQ);
			new_page_meta->page_frame = 0;
			new_page_meta->tid = current->thread_block->tid;
			new_page_meta->next = NULL;
			new_page_meta->head = NULL;
			new_page_meta->middle = NULL;
			new_page_meta->front_meta = NULL;
			new_page_meta->num_pages = 1;
			current->thread_block->first_page = new_page_meta;
			frame_table[0] = new_page_meta;
			current_page_meta = new_page_meta;
		} else {//No free pages in physical memory, check swap file
			//check if swapfile is full	
		}
		head = NULL;
		middle = NULL;
	}
	
	rtn_ptr = mymalloc(x, file, line, myblock_ptr + current->thread_block->malloc_frame*4096, 4096);
	current_page_meta->head = head;
	current_page_meta->middle = middle;
	current_page_meta->free_page_mem = free_mem_count();
	
	//If there was no space in the page for the request...
	if (rtn_ptr == NULL) {
		
		//Check if any of the previous pages has room for the request.
		page_meta_ptr temp_ptr = current->thread_block->first_page;
		int counter = 0;
		while (temp_ptr != NULL) {
			//Can the temp page possibly fit the request?
			if (temp_ptr->free_page_mem >= x) {
				current_page_meta = temp_ptr;
				
				//Is the current page in physical memory? If not swap it in.
				if (current_page_meta->page_frame != counter) {
					swap_pages(current_page_meta->page_frame, counter);
				}
				
				if (current_page_meta->front_meta != NULL) {
					int i;
					page_meta_ptr swap_ptr;
					page_meta_ptr block_temp;
					
					swap_ptr = current_page_meta->front_meta;
					for (i = current_page_meta->front_meta->page_frame; i < current_page_meta->page_frame; i++) {
						block_temp = frame_table[i];
						if (block_temp != swap_ptr) {
							swap_pages(swap_ptr->page_frame, block_temp->page_frame);
						}
						swap_ptr = swap_ptr->next;
					}
				}
				head = current_page_meta->head;
				middle = current_page_meta->middle;
				
				//0s shouldn't matter because head and middle are already defined.
				rtn_ptr = mymalloc(x, file, line, 0, 0);
				if (rtn_ptr != NULL) {
					current_page_meta->head = head;
					current_page_meta->middle = middle;
					current_page_meta->free_page_mem = free_mem_count();
					return rtn_ptr;
				}
			}
			counter++;
			temp_ptr = temp_ptr->next;
		}
		
		
		//If you reach here, then none of the thread's pages could fit the request, so we need to allocate a new page.
		//allocate new page from free list
		if (page_counter < 6004) {
			current->thread_block->malloc_frame++;
			
			//Swaps the page after the current page to free space for contiguous pages
			swap_pages(current->thread_block->malloc_frame, -1);
			new_page_meta = (page_meta_ptr)myallocate(sizeof(struct page_meta), NULL, 0, LIBRARYREQ);
			new_page_meta->page_frame = current->thread_block->malloc_frame;
			new_page_meta->tid = current->thread_block->tid;
			new_page_meta->next = NULL;
			new_page_meta->head = NULL;
			new_page_meta->middle = NULL;
			new_page_meta->num_pages = 1;
			frame_table[current->thread_block->malloc_frame] = new_page_meta;
			frame_table[current->thread_block->malloc_frame-1]->next = new_page_meta;
			current_page_meta = new_page_meta;
		} else {
			//check if swapfile is full
		}
		head = NULL;
		middle = NULL;
		
		rtn_ptr = mymalloc(x, file, line, myblock_ptr + current->thread_block->malloc_frame*4096, 4096);
		current_page_meta->head = head;
		current_page_meta->middle = middle;
		current_page_meta->free_page_mem = free_mem_count();
	}
	return rtn_ptr;
}

int mydeallocate(void * x, char * file, int line, req_type rt){
	
	int mem_offset = (int)(x - myblock_ptr);
	int index = mem_offset / 4096;
	int i;
	
	//Check to see if proper page is in frame. Swap if not
	if (frame_table[index]->tid == current->thread_block->tid) {
		current_page_meta = frame_table[index];
		head = current_page_meta->head;
		middle = current_page_meta->middle;
		
		//Check myfree return value for errors
		myfree(x, file, line);
		current_page_meta->head = head;
		current_page_meta->middle = middle;
		current_page_meta->free_page_mem = free_mem_count();
	} else {
		//Page isn't in the frame, so find it and swap it in
		page_meta_ptr temp;
		
		if (index != 0 && frame_table[index-1]->tid == current->thread_block->tid) {
			temp = frame_table[index-1]->next;
		} else {
			temp = current->thread_block->first_page;
			for (i = 0; i < index; i++) {
				temp = temp->next;
			}
		}
		
		//Found the page, now swap it
		current_page_meta = temp;
		head = current_page_meta->head;
		middle = current_page_meta->middle;
		swap_pages(index, current_page_meta->page_frame);
		
		//Check myfree return value for errors
		myfree(x, file, line);
		current_page_meta->head = head;
		current_page_meta->middle = middle;
		current_page_meta->free_page_mem = free_mem_count();
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

		if(page_counter < 1908){
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
* lseek()  --- zero out 16MG of space??? Added O_CREAT, do we need anything else?????
*
* handle signal and swap correspondingly
*
* handle large allocations of multiple pages, TRICKY
* 		Allocation Done
* 		Still need to account for handling larger pages outside of allocating. ****************
*
* deallocation NOTE: keep track of how much free memory is in the block, member is entered in struct ------- DONE
*
* document and comment code
*
* EXTRA: free space in physical memory 
*
***/


















