#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stream_proc.h>
#include <input1.h>
#include "../apps/tscdf.h"

sid32 sems[501]; //large semaphore array for global init

int time_window, output_time; //global vars for tscdf

typedef struct{ //pair of stream ID, time stamp, and values

	int sid;
	int ts;
	int v;

}pair;

typedef struct{ //dynamic array/stack for pairs
	
	int depth;
	int members;
	pair *data;

} Queue;

Queue queues[500]; //large array of arrays of pairs for global use

void queueCreate(Queue *queue, int workD){ //queue creation, dynamically assigns size to work depth

	queue->depth = workD;
	queue->members = 0;
	queue->data = getmem(sizeof(pair) * queue->depth);

}

void queuePush(Queue *queue, pair stv){ //assumes queue not full, adds to queue

	queue->data[queue->members++] = stv;

}

pair queueGet(Queue *queue, int index){ //assumes index is in range, returns data without removing

	return queue->data[index];

}

bool queueFull(Queue *queue){//checks if queue is full

	return (queue->members == queue->depth);
}

pair queuePop(Queue *queue){ //common pop command

	pair ret = queueGet(queue, 0); //get pair to return
	
	int i;
	for(i = 0; i < queue->members-1; i++){ //shift all members up

		queue->data[i] = queue->data[i+1];
	}

	queue->members--; //update size info

	return ret;
}

void queueFree(Queue *queue){//frees memory of queue

	freemem(queue->data, (sizeof(pair) * queue->depth));

}

void prod(int n_input, int num_streams){//producer process

        int i; //iterator
        int st, ts, v; //temp vars for header parsing
        char* a; //^

        for(i = 0; i<n_input; i++){//iterate through input

                pair tmp;//create temp pair and insatnce with data from input

                a = (char *)stream_input[i];
                st = atoi(a);
                tmp.sid = st;
                while (*a++ != '\t');
                ts = atoi(a);
                tmp.ts = ts;
                while (*a++ != '\t');
                v = atoi(a);
                tmp.v = v;

                if(st < num_streams){//if sid is within our range
                        wait(sems[501]);//ensure that producer can write

			while(queueFull(&queues[st])); //wait if queue is full
                        queuePush(&queues[st], tmp);//push pair
                        signal(sems[st]);//signal sid of stream taking data
                }
        }

		for(i = 0; i < num_streams; i++){//kill consumers with special kill pair

			pair tmp2;
			tmp2.v = -99;

			wait(sems[501]);
			queuePush(&queues[i], tmp2);
			signal(sems[i]);
		}

		return;

}

void cons(int id){//consumer

	struct tscdf *tc = tscdf_init(time_window);//init tscdf
	int tOut = 0;
	int i;

        while(1){
	
			wait(sems[id]);//wait for access to queue
		
			pair p = queuePop(&queues[id]);	//get pair	
		
		if(p.v == -99){//if kill pair, kill self
			signal(sems[501]);
			//queueFree(&queues[id]);
			return;
		}
		else{//else, update tscdf

            //printf("Consume with ID of: %d has value of %d\n", id, p.v);
                	
			tscdf_update(tc, p.ts, p.v);
			tOut++;	

            signal(sems[501]);//done with critical section

		}

		if(tOut==output_time){

			int32 * qarray = tscdf_quartiles(tc);

			if(qarray == NULL) {
    				kprintf("tscdf_quartiles returned NULL\n");
			}

			for(i=0; i < 5; i++) {
   				kprintf("%d ", qarray[i]);
			}
			kprintf("\n");
      
			freemem((char *)qarray, (6*sizeof(int32)));

		}

        }

	return;

}

shellcmd xsh_stream_proc(int nargs, char *args[]) {//main

	//defaults
	int num_streams = 1;
	int work_queue = 10;
	time_window = 25;
	output_time = 10;

	char usage[] = "Usage: -s num_streams -w work_queue -t time_window -o output_time\n";

	if(!(nargs%2)){
		printf("%s, ARG ERR\n", usage);
		return(-1);
	}
	else{
		int i = nargs-1;
		while(i > 0) {
			char* ch = args[i-1];
			char c = *(++ch);
			
			switch(c) {
		
			case 's':
				num_streams = atoi(args[i]);
				break;
			case 'w':
				work_queue = atoi(args[i]);
				break;
			case 't':
				time_window = atoi(args[i]);
				break;
			case 'o':
				output_time = atoi(args[i]);
				break;
			default:
				printf("%s, DEFAULT SWITCH\n", usage);
				return(-1);
			}
		i -= 2;
		}
	}
	//determine n_input 
	int n_input = (sizeof (stream_input) / sizeof (const char *));

	//initialize proper number of queues
	int q;
	for(q = 0; q < num_streams; q++){
		Queue tmp;
		queueCreate(&tmp, work_queue);
		queues[q] = tmp;
	}

	//initialize proper number of stream semaphores
    int j = 0;
    for(j = 0; j < num_streams; j++){
		sems[j] = semcreate(0);
    }
	//initialize special semaphore for producer
	sems[501] = semcreate(0);
    signal(sems[501]);

	//start producer
	resume(create(prod, 1024, 20, "prod", 2, n_input, num_streams));

	//start streams (consumers)
	int s;
	for(s = 0; s < num_streams; s++){
		resume(create(cons, 1024, 20, "consumer", 1, s));
	}
	
	return 0;
}
