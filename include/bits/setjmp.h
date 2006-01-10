/* For internal uses, get the public <bits/setjmp.h> definitions
   plus the JB_* macros from the private header <jmpbuf-offsets.h>.  */

#include_next <bits/setjmp.h>
#ifndef _ISOMAC
# include <jmpbuf-offsets.h>
#endif
