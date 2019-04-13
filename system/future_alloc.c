#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <future.h>


future* future_alloc(int future_flags, uint size){

	intmask mask; //disable interrupts
	mask = disable();

	future *ret; //pointer to return

	ret = (future*)getmem(sizeof(future)); //get mem for struct

	ret->state = FUTURE_EMPTY; //set state to empty
	ret->flags = future_flags; //set flags
	ret->size = size; //set size
	ret->value = getmem(sizeof(char) * size); //allocate space for value
	ret->get_queue = getmem(sizeof(queue));
	get_queue->next = NULL;
	get_queue->getStatus = 0;
	ret->set_queue = getmem(sizeof(queue));
	set_queue->next = NULL;
	set_queue->setStatus = 0;

	restore(mask); //restore mask

	return ret; //return pointer
	
}
