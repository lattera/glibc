#ifndef _AIO_H
#include <rt/aio.h>

/* Now define the internal interfaces.  */
extern void __aio_init (__const struct aioinit *__init);

/* Flag to signal we need to be compatible with glibc < 2.4 in
   lio_listio and we do not issue events for each individual list
   element.  */
#define LIO_NO_INDIVIDUAL_EVENT	128

#endif
