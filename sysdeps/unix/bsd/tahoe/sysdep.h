/* The Tahoe is just like the Vax, except the
   `chmk' instruction is called `kcall'.  */

#define	chmk	kcall
#include <sysdeps/unix/bsd/vax/sysdep.h>
