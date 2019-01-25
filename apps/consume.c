#include <xinu.h>
#include <prodcons.h>
//#include <prodcons.h>

void consumer(int count) {

  // reads the value of the global variable 'n'
  // 'count' times.
  // print consumed value e.g. consumed : 8

  int i;
  for(i = 0; i < count; i++){
	wait(produced); //wait for n to be available for reading
	printf("Consumed: %d\n", n);
	signal(consumed); //signal n has been read
  }
}

