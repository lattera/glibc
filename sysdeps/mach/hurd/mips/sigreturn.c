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

register int sp asm ("$29"), fp asm ("$30");

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
  if (*reply_port)
    __mach_port_destroy (__mach_task_self (), *reply_port);
  *reply_port = scp->sc_reply_port;

  /* Restore registers.  */
#define restore_gpr(n) \
  asm volatile ("lw $" #n ",%0" : : "m" (scpreg->sc_gpr[n]))

  asm volatile (".set noreorder; .set noat;");
  {
    register const struct sigcontext *const scpreg asm ("$1") = scp;

    /* Load the general-purpose registers from the sigcontext.  */
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

    /* Now the special-purpose registers.  */
    sp = scpreg->sc_sp;		/* Stack pointer.  */
    fp = scpreg->sc_fp;		/* Frame pointer.  */
    restore_gpr (31);		/* Return address.  */

    /* Now jump to the saved PC.  */
    asm volatile ("lw $24, %0\n" /* Load saved PC into temporary $t8.  */
		  "j $24\n"	/* Jump to the saved PC value.  */
		  "lw $1, %1\n" /* Restore $at in delay slot.  */
		  : :
		  "m" (scpreg->sc_pc),
		  "m" (scpreg->sc_r1) /* $at */
		  : "$24");	/* XXX clobbers $24 (aka $t8)!! */
    asm volatile (".set reorder; .set at;");
  }

  /* NOTREACHED */
  return -1;
}
