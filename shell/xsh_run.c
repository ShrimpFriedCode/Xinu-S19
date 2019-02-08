#include <xinu.h>
#include <string.h>
#include <stdio.h>

shellcmd xsh_run(int nargs, char *args[]) {

if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0))
    {
      printf("hello\n");
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
}