/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#ifndef _LOWLEVELSEM_H
#define _LOWLEVELSEM_H	1

#ifndef LOCK
# ifdef UP
#  define LOCK	/* nothing */
# else
#  define LOCK "lock;"
# endif
#endif

#define SYS_futex		240


#define lll_sem_wait(sem) \
  ({ int result, ignore1, ignore2;					      \
     __asm __volatile ("1:\tincl 8(%4)\n\t"				      \
		       LOCK "incl (%4)\n\t"				      \
		       "jng 2f\n"					      \
		       ".subsection 1\n"				      \
		       "2:\tmovl %4, %%eax\n\t"				      \
		       "call __lll_unlock_wake\n\t"			      \
		       "jmp 3f\n\t"					      \
		       ".previous\n"					      \
		       "3:\tpushl %%ebx\n\t"				      \
		       "movl %%esi, %%ecx\n\t"				      \
		       "movl %%esi, %%edx\n\t"				      \
		       "leal 4(%4), %%ebx\n\t"				      \
		       "movl %5, %%eax\n\t"				      \
		       "int $0x80\n\t"					      \
		       "movl %%eax, %%edx\n\t"				      \
		       "popl %%ebx\n\t"					      \
		       "orl $-1, %%eax\n\t"				      \
		       LOCK "xaddl %%eax, (%4)\n\t"			      \
		       "jne 4f\n\t"					      \
		       ".subsection 1\n"				      \
		       "4:\tmovl %4, %%ecx\n\t"				      \
		       "call __lll_lock_wait\n\t"			      \
		       "jmp 5f\n\t"					      \
		       ".previous\n"					      \
		       "5:\tdecl 8(%4)\n\t"				      \
		       "xorl %0, %0\n\t"				      \
		       "cmpl $0, 4(%4)\n\t"				      \
		       "jne,pt 6f\n\t"					      \
		       "cmpl %7, %%edx\n\t"				      \
		       "jne,pn 1b\n\t"					      \
		       "addl %8, %0\n\t" /* Shorter than movl %7, %0 */	      \
		       "6:"						      \
		       : "=a" (result), "=c" (ignore1), "=d" (ignore2),	      \
			 "=m" (*sem)					      \
		       : "D" (sem), "i" (SYS_futex), "S" (0),		      \
			 "i" (-EINTR), "i" (EINTR));			      \
     result; })


extern int __lll_sem_timedwait (struct sem *sem, const struct timespec *ts)
     __attribute__ ((regparm (2))) attribute_hidden;
#define lll_sem_timedwait(sem, timeout) \
  __lll_sem_timedwait (sem, timeout)


#define lll_sem_post(sem) \
  (void) ({ int ignore1, ignore2, ignore3;				      \
	    __asm __volatile ("movl $1, %%eax\n\t"			      \
			      LOCK					      \
			      "xaddl %%eax, (%4)\n\t"			      \
			      "pushl %%esi\n\t"				      \
			      "pushl %%ebx\n\t"				      \
			      "movl %4, %%ebx\n\t"			      \
			      "leal 1(%%eax), %%edx\n\t"		      \
			      "xorl %%esi, %%esi\n\t"			      \
			      "movl %5, %%eax\n\t"			      \
			      /* movl $FUTEX_WAKE, %ecx */		      \
			      "movl $1, %%ecx\n\t"			      \
			      "int $0x80\n\t"				      \
			      "popl %%ebx\n\t"				      \
			      "popl %%esi"				      \
			      : "=&a" (ignore1), "=c" (ignore2),	      \
				"=m" (*sem), "=d" (ignore3)		      \
			      : "r" (sem), "i" (SYS_futex)); })

#endif	/* lowlevelsem.h */
