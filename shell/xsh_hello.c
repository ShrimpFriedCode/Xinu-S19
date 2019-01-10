/* xsh_hello.c - xsh_hello */

#include <xinu.h>
#include <string.h>
#include <stdio.h>

shellcmd xsh_hello(int nargs, char *args[]) {

	if(nargs <= 1) {//throw err if too few args
		fprintf(stderr, "Too few arguments.\n", args[0]);
		return 1;
	}
	else if(nargs > 2) {//throw err if too many args
		fprintf(stderr, "Too many arguments.\n", args[0]);
		return 1;
	}
	else//else right amount of args has been passed
		printf("Hello %s, welcome to the world of xinu!\n", args[1]);//print
		return 0;

}
