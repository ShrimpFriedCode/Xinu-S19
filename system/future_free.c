#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <future.h>

syscall future_free(future* f){
	
	intmask mask; //disable interrupts
	mask = disable();

	syscall store, whole; //initialize returns
	
	queue * curr=f->get_queue;
	queue * temp;
	while(curr!=NULL){
		temp=curr->next;
		freemem(curr, (sizeof(queue)))
		curr=temp;
	}
	
	queue * curr2=f->set_queue;
	queue * temp2;
	while(curr2!=NULL){
		temp2=curr2->next;
		freemem(curr2, (sizeof(queue)))
		curr2=temp2;
	}
	
	store = freemem(f->value, (sizeof(char) * f->size)); //free value storage
	whole = freemem((char *)f, (sizeof(future))); //free struct 
	
	if(store == SYSERR || whole == SYSERR){ //ensure no errors were encountered in freeing
		return SYSERR;
	}

	restore(mask);	
	return OK;
	
}
