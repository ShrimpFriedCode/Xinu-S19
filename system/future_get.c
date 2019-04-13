#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <future.h>

void push(queue * head, pid32 pid) {
	queue * current = head;
	while (current->next != NULL) {
		current = current->next;
	}


	current->next = malloc(sizeof(queue));
	current->next->pid = pid;
	current->next->getStatus = 1;
	current->next->next = NULL;
}

pid32 pop(queue ** head) {
	pid32 retval;
	queue * next_node = NULL;

	next_node = (*head)->next;
	retval = (*head)->pid;
	freemem(*head, (sizeof(queue)));
	*head = next_node;

	return retval;
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
			f->get_queue->pid = getpid();//head is empty, so add	
			resched();//reschedule process
			restore(mask);
			return OK;
		}
		else if(f->state == FUTURE_WAITING){
			prptr = &proctab[getpid()];//get process from process table
			prptr->prstate = PR_WAIT; //change process to state wait
			push(f->get_queue, getpid());//push to get queue
			resched();//reschedule process
			restore(mask);
			return OK;
		}
		else if(f->state == FUTURE_FULL){
			memcpy(value, &f->value, sizeof(&f->value));//get value
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
	else if(f->flags == FUTURE_QUEUE){
		if(f->set_queue->setStatus == 1){
			memcpy(value, &f->value, sizeof(&f->value));//get value
			ready(pop(f->set_queue));
			f->get_queue->getStatus == 0;
			restore(mask);
			return OK;
		}
		else if(f->set_queue == NULL || f->set_queue->setStatus == 0 ){
			if(f->get_queue == NULL){
				f->get_queue = getmem(sizeof(queue));
				f->get_queue->next = NULL;
				f->get_queue->getStatus = 1;
				f->get_queue->pid = getpid();
			}
			else if(f->get_queue->getStatus == 0){
				f->get_queue->pid = getpid();
				f->get_queue->getStatus == 1;
			}
			else{
				push(f->get_queue, getpid());
			}
			prptr = &proctab[getpid()];//get process from process table
			prptr->prstate = PR_WAIT; //change process to state wait
			resched();
			restore(mask);
			return OK;
			
		}
	}
	else{
		restore(mask);
		return SYSERR;
	}
	
}
