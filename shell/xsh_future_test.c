#include <xinu.h>
#include <future.h>
#include <stdlib.h>

uint32 future_ring(future *in, future *out);

/**
 *  * Test Futures
 *   */
uint32 future_test(int nargs, char *args[])
{
  int ring = 0;
  int future_flags = 0;
  int ring_count = 10;
  int final_val;
  int i;

#ifndef NFUTURE
  printf("no support for futures (NFUTURES not defined.)\n");
  return OK;
#endif

  if (nargs == 2 && strncmp(args[1], "-r", 2) == 0) {
    ring = 1;
    printf("Producer/consumer process ring, with default size (no ring count provided)\n");
  }
  else if(nargs == 3 && strncmp(args[1], "-r", 2) == 0) {
    ring = 1;
    ring_count = atoi(args[2]);
    printf("Producer/consumer process ring with %d many rings\n", ring_count);
  }
  else if(nargs == 1){
    ring = 1;
    printf("Producer/consumer process ring, with default size (no ring count provided)\n");
  }
  else {
    printf("No valid options\n");
    return(OK);
  }

  if (ring == 1) {
    future *first, *in, *out = NULL;
    first = future_alloc(future_flags, sizeof(int));
    in = first;
	
    for (i=0; i < ring_count; i++) {
      out = future_alloc(future_flags, sizeof(int));
      resume( create(future_ring, 1024, 20, "", 2, in, out) );
      in = out;
    }
	
    printf("master sets %d\n", ring_count);
    future_set(first, (char *)&ring_count);
    future_get(out, (char *)&final_val);
    printf("master gets %d\n", final_val);

    return(OK);
  }
  return(OK);
}

uint future_ring(future *in, future *out) {
  int val;
  future_get(in, (char *)&val);
  printf("Process %d gets %d, puts %d\n", getpid(), val, val-1);
  val--;
  future_free(in);
  future_set(out, (char *)&val);
  return OK;
}

shellcmd xsh_future_test(int nargs, char *args[]){
	
	return (future_test(nargs, args));

}
