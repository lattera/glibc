#include_next <pthread.h>

/* This function is called to initialize the pthread library.  */
extern void __pthread_initialize (void) __attribute__ ((weak));

extern void __pthread_initialize_minimal (void) __attribute__ ((weak));
