/* umount system call with two parameters.  */

#include <sysdep.h>
#if defined __NR_oldumount || defined __NR_umount2
#ifdef __NR_oldumount
PSEUDO (__umount2, umount, 2)
#else
PSEUDO (__umount2, umount2, 2)
#endif
	ret
PSEUDO_END(__umount2)
weak_alias (__umount2, umount2)
#endif
