#include_next <pthread.h>

#ifndef _ISOMAC
/* This function is called to initialize the pthread library.  */
extern void __pthread_initialize (void) __attribute__ ((weak));
#endif
