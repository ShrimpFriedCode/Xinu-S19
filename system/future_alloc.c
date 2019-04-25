#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <future.h>

queue *createQueue() 
{ 
    queue *q = (queue*)getmem(sizeof(queue)); 
    q->first = q->last = NULL; 
    return q; 
} 

future* future_alloc(int future_flags, uint size){

	intmask mask; //disable interrupts
	mask = disable();

	future *ret; //pointer to return

	ret = (future*)getmem(sizeof(future)); //get mem for struct

	ret->state = FUTURE_EMPTY; //set state to empty
	ret->flags = future_flags; //set flags
	ret->size = size; //set size
	ret->value = (char *)getmem(size); //allocate space for value
	queue* set = createQueue();
	queue* get = createQueue();

	ret->get_queue = get;
	ret->set_queue = set;

	restore(mask); //restore mask

	return ret; //return pointer
	
}
