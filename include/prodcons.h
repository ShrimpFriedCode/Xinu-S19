/*Global variable for proucer consumer*/
extern int n; /*this is just declaration*/

extern sid32 produced, consumed; /* declaration for semaphore */

/*function Prototype*/
void consumer(int count);
void producer(int count);
