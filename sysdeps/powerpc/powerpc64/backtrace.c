/* Return backtrace of current program state.
   Copyright (C) 1998-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <execinfo.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <libc-vdso.h>

/* This is the stack layout we see with every stack frame.
   Note that every routine is required by the ABI to lay out the stack
   like this.

            +----------------+        +-----------------+
    %r1  -> | %r1 last frame--------> | %r1 last frame--->...  --> NULL
            |                |        |                 |
            | cr save        |        | cr save	  |
            |                |        |                 |
            | (unused)       |        | return address  |
            +----------------+        +-----------------+
*/
struct layout
{
  struct layout *next;
  long condition_register;
  void *return_address;
};

/* Since the signal handler is just like any other function it needs to
   save/restore its LR and it will save it into callers stack frame.
   Since a signal handler doesn't have a caller, the kernel creates a
   dummy frame to make it look like it has a caller.  */
struct signal_frame_64 {
#define SIGNAL_FRAMESIZE 128
  char            dummy[SIGNAL_FRAMESIZE];
  struct ucontext uc;
  /* We don't care about the rest, since the IP value is at 'uc' field.  */
};

static inline int
is_sigtramp_address (unsigned long nip)
{
#ifdef SHARED
  if (nip == (unsigned long)__vdso_sigtramp_rt64)
    return 1;
#endif
  return 0;
}

int
__backtrace (void **array, int size)
{
  struct layout *current;
  int count;

  /* Force gcc to spill LR.  */
  asm volatile ("" : "=l"(current));

  /* Get the address on top-of-stack.  */
  asm volatile ("ld %0,0(1)" : "=r"(current));

  for (				count = 0;
       current != NULL && 	count < size;
       current = current->next, count++)
    {
      array[count] = current->return_address;

      /* Check if the symbol is the signal trampoline and get the interrupted
       * symbol address from the trampoline saved area.  */
      if (is_sigtramp_address ((unsigned long)current->return_address))
        {
	  struct signal_frame_64 *sigframe = (struct signal_frame_64*) current;
          array[++count] = (void*)sigframe->uc.uc_mcontext.gp_regs[PT_NIP];
	}
    }

  /* It's possible the second-last stack frame can't return
     (that is, it's __libc_start_main), in which case
     the CRT startup code will have set its LR to 'NULL'.  */
  if (count > 0 && array[count-1] == NULL)
    count--;

  return count;
}
weak_alias (__backtrace, backtrace)
libc_hidden_def (__backtrace)
