#include <xinu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <future.h>

syscall future_set(future *f, char* value){

	intmask	mask; //disable interrupts
	mask = disable();	

	if(f->state == FUTURE_FULL){//if future is full, error
		restore(mask);
		return SYSERR;
	}
	else if(f->state == FUTURE_EMPTY){//if empty, insert
		f->state = FUTURE_FULL; //change state to full
		memcpy(&f->value, value, (sizeof(value))); //give value
		restore(mask);
		return OK;
	}
	else if(f->state == FUTURE_WAITING){//if waiting, insert and wake
		memcpy(&f->value, value, (sizeof(value)));//give value
        f->state = FUTURE_FULL;//change state
		ready(f->pid);//ready waiting process, and resched
		restore(mask);
		return OK;
	}
	else{//else state unkown and error
		restore(mask);
		return SYSERR;
	}

	restore(mask);
	return SYSERR;
}
