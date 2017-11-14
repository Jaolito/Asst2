#include "mymemorymanager.h"

static char myblock[8388608];
static char free_list_memory[sizeof(struct page_meta)*2048];
static int page_table[2048];
char first_call = 1;
context_node * current;
mementryPtr head;
mementryPtr middle;

page_meta_ptr free_queue_head, free_queue_back;

void * myallocate(unsigned int x, char * file, int line, req_type rt) {
	
	page_meta_ptr rtn_page;
	
	if (first_call) {
		first_call = 0;
		
		int i;
		
		for (i = 0; i < 2048; i++) {
			page_meta_ptr temp = (page_meta_ptr)mymalloc(sizeof(struct page_meta), NULL, 0, free_list_memory, sizeof(struct page_meta)*2048);
			temp->page_frame = i;
			temp->next = NULL;
			page_enqueue(temp);
		}
	}
	
	if (current->thread_block->has_page) {
		head = current->thread_block->head;
		middle = current->thread_block->middle;
	} else {
		head = NULL;
		middle = NULL;
	}
	
	if (free_queue_head != NULL) {
		rtn_page = page_dequeue();
		page_table[rtn_page->page_frame] = rtn_page->page_frame;
	} else {
		//check if swapfile is full
	}
	current->thread_block->has_page = '1';
	
	void * rtn_ptr = mymalloc(x, file, line, myblock + rtn_page->page_frame*4096, 4096);
	if (rtn_ptr == NULL) {
		//allocate new page
		if (free_queue_head != NULL) {
			rtn_page = page_dequeue();
		} else {
			//check if swapfile is full
		}
		head = NULL;
		middle = NULL;
		rtn_ptr = mymalloc(x, file, line, myblock + rtn_page->page_frame*4096, 4096);
	}
	return rtn_ptr;
	
	
}

int mydeallocate(void * x, char * file, int line, req_type rt){
	
	head = current->thread_block->head;
	middle = current->thread_block->middle;
	
	//Check to see if proper page is in frame. Swap if not
	
	
	//Check myfree return value for errors
	myfree(x, file, line);
	
	//How do we tell a thread that one of its pages has free space if it has multiple pages
	
	/*
	 * Strings as a way to keep track of what pages each thread has?
	string = "" + 1 + "," + 5;
	
	string = string + "," + 15;
	is this allowed???
	*/
	
	return 0;
}

int main(){
	return 0;
}

/* Helper Functions */
void page_enqueue(page_meta_ptr node){
	free_queue_back->next = node;
	free_queue_back = node;
}

page_meta_ptr page_dequeue(){
	page_meta_ptr temp = free_queue_head;
	free_queue_head = free_queue_head->next;
	temp->next = NULL;
	return temp;
}
