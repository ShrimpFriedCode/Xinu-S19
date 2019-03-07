#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <future.h>

syscall future_free(future* f){
	
	intmask mask; //disable interrupts
	mask = disable();

	syscall store, whole; //initialize returns
	
	store = freemem(f->value, (sizeof(char) * f->size)); //free value storage
	whole = freemem((char *)f, (sizeof(future))); //free struct 
	
	if(store == SYSERR || whole == SYSERR){ //ensure no errors were encountered in freeing
			return SYSERR;
	}
	
	return OK;
	
}
