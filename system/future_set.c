#include<xinu.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<future.h>

void push(queue * head, pid32 pid) {
    queue * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(queue));
    current->next->pid = pid;
	current->next->setStatus = 1;
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

syscallfuture_set(future*f,char*value){
	intmaskmask;//disable interrupts
	mask=disable();
	
	struct  procent *prptr;
	
	if(f->flags==FUTURE_SHARED){
		
		if(f->state==FUTURE_FULL){//if future is full, error
			restore(mask);
			return SYSERR;
		}
		else if(f->state==FUTURE_EMPTY){//if empty, insert
			f->state=FUTURE_FULL;//change state to full
			memcpy(&f->value,value,(sizeof(value)));//give value
			
			queue * curr=f->get_queue;
			while(curr!=NULL)
			{
				ready(curr->pid);
				curr=curr->next;
			}
			
			restore(mask);
			return OK;
		}
		else if(f->state==FUTURE_WAITING){//if waiting, insert and wake
			memcpy(&f->value,value,(sizeof(value)));//give value
			f->state=FUTURE_FULL;//change state
			queue * curr=f->get_queue;
			while(curr!=NULL){
				ready(curr->pid);
				curr=curr->next;
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
			memcpy(&f->value,value,(sizeof(value)));//give value
			restore(mask);
			return OK;
		}
		else if(f->state==FUTURE_WAITING){//if waiting, insert and wake
			memcpy(&f->value,value,(sizeof(value)));//give value
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
		if(f->get_queue == NULL || f->get_queue->getStatus == 0){
				memcpy(&f->value,value,(sizeof(value)));//give value
				if(f->set_queue == NULL){
					f->set_queue = getmem(sizeof(queue));
					f->set_queue->next = NULL;
					f->set_queue->setStatus = 1;
					f->set_queue->pid = getpid();
				}
				else if(f->set_queue->setStatus == 0){
						f->set_queue->pid = getpid();
						f->set_queue->setStatus == 1;
				}
				else{
						push(f->set_queue, getpid());
				}
				prptr = &proctab[getpid()];//get process from process table
                prptr->prstate = PR_WAIT; //change process to state wait
				resched();
				restore(mask);
				return OK;
		}
		if(f->get_queue->getStatus == 1){
				memcpy(&f->value,value,(sizeof(value)));//give value
				ready(pop(f->get_queue));
				f->set_queue->setStatus = 0;
				restore(mask);
				return OK;
		}
	}
	else{
		restore(mask);
		return(SYSERR);
	}
}