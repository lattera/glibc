/* This hack is necessary since the kernel people are making "strange"
   changes.  They simply rename old system calls.  */

#include <sysdep.h>
#ifdef __NR_oldumount
PSEUDO (__umount, oldumount, 1)
#else
PSEUDO (__umount, umount, 1)
#endif
	ret
PSEUDO_END(__umount)
weak_alias (__umount, umount)
