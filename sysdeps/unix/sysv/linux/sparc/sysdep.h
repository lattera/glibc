/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2000.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LINUX_SPARC_SYSDEP_H
#define _LINUX_SPARC_SYSDEP_H 1

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) inline_syscall##nr(name, args)

#define inline_syscall0(name,dummy...)					\
({									\
	register long __o0 __asm__ ("o0");				\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm__ (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :		\
		 "0" (__g1) :						\
		 __SYSCALL_CLOBBERS);					\
	__o0;								\
})

#define inline_syscall1(name,arg1)					\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm__ (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :		\
		 "0" (__g1), "1" (__o0) :				\
		 __SYSCALL_CLOBBERS);					\
	__o0;								\
})

#define inline_syscall2(name,arg1,arg2)					\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm__ (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :		\
		 "0" (__g1), "1" (__o0), "r" (__o1) :			\
		 __SYSCALL_CLOBBERS);					\
	__o0;								\
})

#define inline_syscall3(name,arg1,arg2,arg3)				\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm__ (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :		\
		 "0" (__g1), "1" (__o0), "r" (__o1), "r" (__o2) :	\
		 __SYSCALL_CLOBBERS);					\
	__o0;								\
})

#define inline_syscall4(name,arg1,arg2,arg3,arg4)			\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm__ (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :		\
		 "0" (__g1), "1" (__o0), "r" (__o1), "r" (__o2),	\
		 "r" (__o3) :						\
		 __SYSCALL_CLOBBERS);					\
	__o0;								\
})

#define inline_syscall5(name,arg1,arg2,arg3,arg4,arg5)			\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm__ (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :		\
		 "0" (__g1), "1" (__o0), "r" (__o1), "r" (__o2),	\
		 "r" (__o3), "r" (__o4) :				\
		 __SYSCALL_CLOBBERS);					\
	__o0;								\
})

#define inline_syscall6(name,arg1,arg2,arg3,arg4,arg5,arg6)		\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	register long __o5 __asm__ ("o5") = (long)(arg6);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm__ (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :		\
		 "0" (__g1), "1" (__o0), "r" (__o1), "r" (__o2),	\
		 "r" (__o3), "r" (__o4), "r" (__o5) :			\
		 __SYSCALL_CLOBBERS);					\
	__o0;								\
})

#endif /* _LINUX_SPARC_SYSDEP_H */
