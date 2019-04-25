#include<xinu.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<future.h>

node* newNodeS(pid32 p) 
{ 
    node *temp = (node*)getmem(sizeof(node)); 
    temp->pid = p; 
    temp->next = NULL; 
    return temp;  
} 

void pushS(queue * q, pid32 p) {
	// Create a new LL node 
    struct node *temp = newNodeS(p); 
  
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

pid32 popS(queue * q) {
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

syscall future_set(future*f,char*value){
	int mask;//disable interrupts
	mask=disable();
	
	struct  procent *prptr;
	
	if(f->flags==FUTURE_SHARED){
		
		if(f->state==FUTURE_FULL){//if future is full, error
			restore(mask);
			return SYSERR;
		}
		else if(f->state==FUTURE_EMPTY){//if empty, insert
			f->state=FUTURE_FULL;//change state to full
			memcpy(f->value,value,f->size);//give value
			restore(mask);
			return OK;
		}
		else if(f->state==FUTURE_WAITING){//if waiting, insert and wake
			memcpy(f->value,value,f->size);//give value
			f->state=FUTURE_FULL;//change state
			
			while(f->get_queue->last!=NULL){
				ready(popS(f->get_queue));
			}
			
			restore(mask);
			return OK;
		}
		else{
			restore(mask);
			return(SYSERR);
		}
		
	}
	else if(f->flags==FUTURE_EXCLUSIVE){
		if(f->state==FUTURE_FULL){//if future is full, error
			restore(mask);
			return SYSERR;
		}
		else if(f->state==FUTURE_EMPTY){//if empty, insert
			f->state=FUTURE_FULL;//change state to full
			memcpy(f->value,value,f->size);//give value
			restore(mask);
			return OK;
		}
		else if(f->state==FUTURE_WAITING){//if waiting, insert and wake
			memcpy(f->value,value,f->size);//give value
			f->state=FUTURE_FULL;//change state
			ready(f->pid);//ready waiting process, and resched
			restore(mask);
			return OK;
		}
		else{
			restore(mask);
			return(SYSERR);
		}
		
	}
	else if(f->flags==FUTURE_QUEUE){
		if(f->get_queue->last == NULL){
			prptr = &proctab[getpid()];//get process from process table
			prptr->prstate = PR_WAIT; //change process to state wait
			pushS(f->set_queue, getpid());
			resched();
			memcpy(f->value,value,f->size);//give value
			restore(mask);
			return OK;
		}
		else{
			memcpy(f->value,value,f->size);//give value
			ready(popS(f->get_queue));
			restore(mask);
			return OK;
		}
	}
	else{
		restore(mask);
		return(SYSERR);
	}
}
