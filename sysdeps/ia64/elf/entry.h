#ifndef __ASSEMBLY__
extern void _start (void);
#endif

/* The function's entry point is stored in the first word of the
   function descriptor (plabel) of _start().  */
#define ENTRY_POINT (((long int *) _start)[0])
