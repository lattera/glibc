/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <hurd.h>
#include <hurd/signal.h>
#include <hurd/threadvar.h>

int
__sigreturn (const struct sigcontext *scp)
{
  struct hurd_sigstate *ss;
  mach_port_t *reply_port;

  if (scp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  ss = _hurd_self_sigstate ();
  ss->blocked = scp->sc_mask;
  ss->intr_port = scp->sc_intr_port;
  if (scp->sc_onstack)
    ss->sigaltstack.ss_flags &= ~SA_ONSTACK; /* XXX threadvars */
  __mutex_unlock (&ss->lock);

  /* Destroy the MiG reply port used by the signal handler, and restore the
     reply port in use by the thread when interrupted.  */
  reply_port =
    (mach_port_t *) __hurd_threadvar_location (_HURD_THREADVAR_MIG_REPLY);
  if (*reply_port != MACH_PORT_NULL)
    __mach_port_destroy (__mach_task_self (), *reply_port);
  *reply_port = scp->sc_reply_port;

  /* Load all the registers from the sigcontext.  */
#define restore_gpr(n) \
  asm volatile ("lw $" #n ",%0" : : "m" (scpreg->sc_gpr[n - 1]))

  {
    register const struct sigcontext *const scpreg asm ("$1") = scp;

    /* Just beyond the top of the user stack, store the user's value for $1
       (which we are using for SCPREG).  We restore this register as the
       very last thing, below.  */
    ((int *) scpreg->sc_gpr[29 - 1])[-1] = scpreg->sc_gpr[0];

    /* First restore the multiplication result registers.  The compiler
       will use some temporary registers, so we do this before restoring
       the general registers.  */
    asm volatile ("mtlo %0" : : "r" (scpreg->sc_mdlo));
    asm volatile ("mthi %0" : : "r" (scpreg->sc_mdhi));

    asm volatile (".set noreorder; .set noat;");

    /* Restore the normal registers.  */
    restore_gpr (2);
    restore_gpr (3);
    restore_gpr (4);
    restore_gpr (5);
    restore_gpr (6);
    restore_gpr (7);
    restore_gpr (8);
    restore_gpr (9);
    restore_gpr (10);
    restore_gpr (11);
    restore_gpr (12);
    restore_gpr (13);
    restore_gpr (14);
    restore_gpr (15);
    restore_gpr (16);
    restore_gpr (17);
    restore_gpr (18);
    restore_gpr (19);
    restore_gpr (20);
    restore_gpr (21);
    restore_gpr (22);
    restore_gpr (23);
    restore_gpr (24);
    restore_gpr (25);
    /* Registers 26-27 are kernel-only.  */
    restore_gpr (28);
    restore_gpr (29);		/* Stack pointer.  */
    restore_gpr (30);		/* Frame pointer.  */
    restore_gpr (31);		/* Return address.  */

    /* Now jump to the saved PC.  */
    asm volatile ("lw $1, %0\n"	/* Load saved PC into $1.  */
		  "j $1\n"	/* Jump to the saved PC value.  */
		  "lw $1, -4(sp)\n" /* Restore $1 from stack in delay slot.  */
		  : : "m" (scpreg->sc_pc));

    asm volatile (".set reorder; .set at;");
  }

  /* NOTREACHED */
  return -1;
}
