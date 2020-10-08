/*
	priority queue implementation

	author: amit vasudevan (amitvasudevan@acm.org)
*/

#include <types.h>

#define PRIORITY_QUEUE_SIZE 5

typedef struct {
	void *data; //data or pointer to data
	int priority; 	//priority (higher value = higher priority)
} priority_queue_t;

__attribute__((section(".data"))) priority_queue_t priority_queue[PRIORITY_QUEUE_SIZE];
__attribute__((section(".data"))) int priority_queue_totalelems = 0;

//maintain priority queue in descending order

// Function to check priority and place element
void check_and_insert(void *data, int priority){
	int i, j;

	for(i=0; i < priority_queue_totalelems; i++){
		if(priority > priority_queue[i].priority){
			//we found the index at which to insert (i)
			//move elements from i through priority_queue_totalelems-1 forward
			for(j=(priority_queue_totalelems-1); j>=i; j--){
				priority_queue[j+1].data = priority_queue[j].data;
				priority_queue[j+1].priority = priority_queue[j].priority;
			}
			//now insert at i
			priority_queue[i].data = data;
			priority_queue[i].priority = priority;
			return;
		}
	}

    priority_queue[i].data = data;
    priority_queue[i].priority = priority;
}

//return 0 on error, 1 on success
int priority_queue_insert(void *data, int priority){
	//return error if we are maxed out
	if(priority_queue_totalelems >= PRIORITY_QUEUE_SIZE ){
		//_XDPRINTF_("%s,%u: Queue overflow, no more elements can be inserted!\n", __func__, __LINE__);
		return 0;
    }

	//if we have no elements then just plug value and priority as the first
	if(priority_queue_totalelems == 0){
		priority_queue[priority_queue_totalelems].data = data;
		priority_queue[priority_queue_totalelems].priority = priority;
		priority_queue_totalelems++;
	}else{
		//we have some elements so check and insert
		check_and_insert(data, priority);
		priority_queue_totalelems++;
	}

	return 1;
}



//return 0 on error, 1 on success
int priority_queue_remove(void *data, int *priority){
	int i;

	//return error if we have no elements
	if(priority_queue_totalelems == 0 ){
		//_XDPRINTFSMP_("%s,%u: No elements in queue!\n", __func__, __LINE__);
		return 0;
    }

	//return the top element
	*((u32 *)data) = priority_queue[0].data;
	*priority = priority_queue[0].priority;

	//move up everything else
	for(i=0; i < (priority_queue_totalelems-1); i++){
		priority_queue[i].data = priority_queue[i+1].data;
		priority_queue[i].priority = priority_queue[i+1].priority;
	}

	priority_queue_totalelems--;
	return 1;
}

//returns 1 if queue is empty else 0
int priority_queue_isempty(void){
	if(priority_queue_totalelems == 0 )
		return 1;
	else
		return 0;
}


#if 0
void priority_queue_display(void){
	int i;

	_XDPRINTF_("%s,%u: Dumping queue...\n", __func__, __LINE__);

	for(i=0; i < priority_queue_totalelems; i++){
		_XDPRINTF_("  index=%u: priority=%d, data=%u\n", i, priority_queue[i].priority,
					priority_queue[i].data);
	}

	_XDPRINTF_("%s,%u: Done.\n", __func__, __LINE__);
}
#endif



