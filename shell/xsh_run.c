#include <xinu.h>
#include <string.h>
#include <stdio.h>

shellcmd xsh_run(int nargs, char *args[]) {

if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0))
    {
      printf("hello\n");
      printf("prodcons\n");
      printf("stream_proc\n");
      printf("future_test\n");
      return OK;
    }

    args++;
    nargs--;

    if(strncmp(args[0], "hello", 5) == 0) {
      xsh_hello(nargs, args);
    }
    else if(strncmp(args[0], "prodcons", 8) == 0) {
      xsh_prodcons(nargs, args);
    }
    else if(strncmp(args[0], "stream_proc", 11) == 0){
      xsh_stream_proc(nargs, args);
    } 
    else if(strncmp(args[0], "future_test", 11) == 0){
      xsh_future_test(nargs, args);
    }
    
   return OK;
}
