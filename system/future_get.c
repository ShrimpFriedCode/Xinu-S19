#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <future.h>

node* newNode(pid32 p) 
{ 
    node *temp = (node*)getmem(sizeof(node)); 
    temp->pid = p; 
    temp->next = NULL; 
    return temp;  
} 

void push(queue * q, pid32 p) {
	// Create a new LL node 
    struct node *temp = newNode(p); 
  
    // If queue is empty, then new node is front and rear both 
    if (q->last == NULL) 
    { 
       q->first = q->last = temp; 
       return; 
    } 
  
    // Add the new node at the end of queue and change rear 
    q->last->next = temp; 
    q->last = temp; 
}

pid32 pop(queue * q) {
// If queue is empty, return NULL. 
    if (q->first == NULL) 
       return NULL; 
  
    // Store previous front and move front one node ahead 
	pid32 ret = q->first->pid; 
    q->first = q->first->next; 
  
    // If front becomes NULL, then change rear also as NULL 
    if (q->first == NULL) 
       q->last = NULL; 
    return ret; 
}

syscall future_get(future* f, char* value){

	intmask	mask;//disable interrupts
	mask = disable();
	struct  procent *prptr; 
	
	if(f->flags == FUTURE_SHARED){
		
		if(f->state == FUTURE_EMPTY){//if empty, wait
			f->state = FUTURE_WAITING;//change state
			prptr = &proctab[getpid()];//get process from process table
			prptr->prstate = PR_WAIT; //change process to state wait
			push(f->get_queue, getpid());
			resched();//reschedule process
			memcpy(value, f->value, f->size);//get value
			restore(mask);
			return OK;
		}
		else if(f->state == FUTURE_WAITING){
			prptr = &proctab[getpid()];//get process from process table
			prptr->prstate = PR_WAIT; //change process to state wait
			push(f->get_queue, getpid());
			resched();//reschedule process
			memcpy(value, f->value, f->size);//get value
			restore(mask);
			return OK;
		}
		else if(f->state == FUTURE_FULL){
			memcpy(value, f->value, f->size);//get value
			restore(mask);
			return OK;

		}
		else if(f->state == FUTURE_FREE){
			restore(mask);
			return SYSERR;
		}

		restore(mask);
		return SYSERR;
	}
	else if(f->flags == FUTURE_EXCLUSIVE){

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
			f->state == FUTURE_EMPTY;
			memcpy(value, f->value, f->size);//get value
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
	else if(f->flags == FUTURE_QUEUE){
		if(f->set_queue->last == NULL){
			prptr = &proctab[getpid()];//get process from process table
			prptr->prstate = PR_WAIT; //change process to state wait
			push(f->get_queue, getpid());
			resched();
			memcpy(value, f->value, f->size);//get value
			restore(mask);
			return OK;
		}
		else{
			ready(pop(f->set_queue));
			memcpy(value, f->value, f->size);//get value
			restore(mask);
			return OK;
		}
	}
	else{
		restore(mask);
		return SYSERR;
	}
	
}
