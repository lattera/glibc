#ifndef _SYS_MMAN_H
#include <misc/sys/mman.h>

/* Now define the internal interfaces.  */
extern __ptr_t __mmap __P ((__ptr_t __addr, size_t __len, int __prot,
			    int __flags, int __fd, __off_t __offset));
extern __ptr_t __mmap64 __P ((__ptr_t __addr, size_t __len, int __prot,
			      int __flags, int __fd, __off64_t __offset));
extern int __munmap __P ((__ptr_t __addr, size_t __len));
extern int __mprotect __P ((__ptr_t __addr, size_t __len, int __prot));

/* This one is Linux specific.  */
extern __ptr_t __mremap __P ((__ptr_t __addr, size_t __old_len,
			    size_t __new_len, int __may_move));
#endif
