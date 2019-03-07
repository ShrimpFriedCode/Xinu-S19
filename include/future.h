#ifndef _FUTURE_H_
#define _FUTURE_H_
 
/* define states */
#define FUTURE_FREE	  0
#define FUTURE_EMPTY 	  1
#define FUTURE_FULL 	  3
#define FUTURE_WAITING  5

/* modes of operation for future*/
#define FUTURE_EXCLUSIVE  0	

typedef struct futent
{
  uint16 state;
  uint16 flags;
  uint32 size;
  char *value; /* alloc must memget size (in chars) space */
  pid32 pid; /* for single waiter case */
} future;

/* Interface for system call */
future* future_alloc(int future_flags, uint size);
syscall future_free(future*);
syscall future_get(future*, char *);
syscall future_set(future*, char *);
 
#endif /* _FUTURE_H_ */
