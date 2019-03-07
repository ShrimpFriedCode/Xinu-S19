#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <future.h>


future* future_alloc(int future_flags, uint size){

	intmask mask; //disable interrupts
	mask = disable();

	if(future_flags != FUTURE_EXCLUSIVE){//only support for exclusive operation is available at this point
		restore(mask);
		return NULL;//no pointer to give
	}

	future *ret; //pointer to return

	ret = (future*)getmem(sizeof(future)); //get mem for struct

	ret->state = FUTURE_EMPTY; //set state to empty
	ret->flags = future_flags; //set flags
	ret->size = size; //set size
	ret->value = getmem(sizeof(char) * size); //allocate space for value

	restore(mask); //restore mask

	return ret; //return pointer
	
}
