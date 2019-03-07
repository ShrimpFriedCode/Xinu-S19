#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <future.h>

syscall future_get(future* f, char* value){

	intmask	mask;//disable interrupts
	mask = disable();
	struct  procent *prptr; 
	
	if(f->state == FUTURE_EMPTY){//if empty, wait
		f->pid = getpid();//store pid
		f->state = FUTURE_WAITING;//change state
		prptr = &proctab[getpid()];//get process from process table
        prptr->prstate = PR_WAIT; //change process to state wait
		resched();//reschedule process
	}

	if(f->state == FUTURE_WAITING){//if waiting, throw error
		restore(mask);
		return SYSERR; 
	}
	else if(f->state == FUTURE_FULL){//if full, get value
		memcpy(value, &f->value, sizeof(&f->value));//get value
		f->pid = getpid();//store pid
		restore(mask);
		return OK;
	}
	else if(f->state == FUTURE_FREE){//if free, throw error
		restore(mask);
		return SYSERR;
	}

	restore(mask);
	return SYSERR;
}
