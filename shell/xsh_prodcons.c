#include <xinu.h>
#include <prodcons.h>

int n;                 //Definition for global variable 'n'
/*Now global variable n will be on Heap so it is accessible all the processes i.e. consume and produce*/

sid32 produced, consumed; //definition for global semaphore lock

shellcmd xsh_prodcons(int nargs, char *args[])
{
  //Argument verifications and validations
  int count = 2000;             //local varible to hold count

  if(nargs > 2){//print error for too many args
	fprintf(stderr, "Too many arguments.\n", args[0]);
	return 1;
  }
  else if(nargs == 2){//get arg, check to see if num
	if(atoi(args[1])){
		count = atoi(args[1]);
	}
	else{//not a number, error
		fprintf(stderr, "Invalid argument. Not a number.\n", args[0]);
	}
  }
  else{
	//default to 2000
  }
 
  //check args[1] if present assign value to count
  //        
  //create the process producer and consumer and put them in ready queue.
  //Look at the definations of function create and resume in the system folder for reference.      
  consumed = semcreate(1); //consumer semaphore
  produced = semcreate(0); //producer semaphore, start with producer
  resume( create(producer, 1024, 20, "producer", 1, count));
  resume( create(consumer, 1024, 20, "consumer", 1, count));
  return (0);
}
