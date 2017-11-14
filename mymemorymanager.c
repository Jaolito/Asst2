#include "mymemorymanager.h"

static char myblock[8388608];
static char free_list_memory[sizeof(page_meta)*2048];
static int page_table[2048];
char first_call = 1;

page_meta_ptr free_queue_head, free_queue_back;

void * myallocate(unsigned int x, char * file, int line, req_type rt) {
	
	page_meta_ptr rtn_page;
	
	if (first_call) {
		first_call = 0;
		
		int i;
		
		for (i = 0; i < 2048; i++) {
			page_meta_ptr temp = (page_meta_ptr)mymalloc(sizeof(page_meta), NULL, 0, free_list_memory, sizeof(page_meta)*2048);
			temp->page_frame = i;
			temp->next = NULL;
			page_enqueue(temp);
		}
	}
	
	
	
	
	
	if (free_queue_head != NULL) {
		rtn_page = page_dequeue();
	} else {
		//check if swapfile is full
	}
	
	
}

int mydeallocate(void * x, char * file, int line, req_type rt){
	
}

int main(){
	
}

/* Helper Functions */
void page_enqueue(page_meta_ptr node){
	free_queue_back->next = node;
	free_queue_back = node;
}

page_meta_ptr page_dequeue(){
	page_meta_ptr temp = free_queue_head;
	free_queue_head = free_queue_head->next;
	return temp;
}
