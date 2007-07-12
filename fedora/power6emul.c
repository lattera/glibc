/* Emulate power6 mf[tf]gpr and fri[zpmn] instructions.
   Copyright (C) 2006 Red Hat, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2006.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   It is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <signal.h>
#include <stdio.h>

extern double frip (double), friz (double), frin (double), frim (double);
asm (".globl frip, friz, frin, frim\n.hidden frip, friz, frin, frim\n\t"
#ifdef __powerpc64__
	".section \".toc\",\"aw\"\n"
"8:"	".tc FD_43300000_0[TC],0x4330000000000000\n"
"9:"	".tc FD_3fe00000_0[TC],0x3fe0000000000000\n\t"
	".previous\n\t"
#else
	".rodata\n\t"
	".align 2\n"
"8:"	".long 0x59800000\n"
"9:"	".long 0x3f000000\n\t"
	".previous\n\t"
#endif
	"# frip == ceil\n"
"frip:"	"mffs    11\n\t"
#ifdef __powerpc64__
	"lfd     13,8b@toc(2)\n\t"
#else
	"mflr    11\n\t"
	"bcl     20,31,1f\n"
"1:"	"mflr    9\n\t"
	"addis   9,9,8b-1b@ha\n\t"
	"lfs     13,8b-1b@l(9)\n\t"
	"mtlr    11\n\t"
#endif
	"fabs    0,1\n\t"
	"fsub    12,13,13\n\t"
	"fcmpu   7,0,13\n\t"
	"fcmpu   6,1,12\n\t"
	"bnllr-  7\n\t"
	"mtfsfi  7,2\n\t"
	"ble-    6,2f\n\t"
	"fadd    1,1,13\n\t"
	"fsub    1,1,13\n\t"
	"fabs    1,1\n\t"
	"mtfsf   0x01,11\n\t"
	"blr\n"
"2:"	"bge-    6,3f\n\t"
	"fsub    1,1,13\n\t"
	"fadd    1,1,13\n\t"
	"fnabs   1,1\n"
"3:"	"mtfsf   0x01,11\n\t"
	"blr\n\t"
	"# friz == trunc\n"
"friz:"	"mffs    11\n\t"
#ifdef __powerpc64__
	"lfd     13,8b@toc(2)\n\t"
#else
	"mflr    11\n\t"
	"bcl     20,31,1f\n"
"1:"	"mflr    9\n\t"
	"addis   9,9,8b-1b@ha\n\t"
	"lfs     13,8b-1b@l(9)\n\t"
	"mtlr    11\n\t"
#endif
	"fabs    0,1\n\t"
	"fsub    12,13,13\n\t"
	"fcmpu   7,0,13\n\t"
	"fcmpu   6,1,12\n\t"
	"bnllr-  7\n\t"
	"mtfsfi  7,1\n\t"
	"ble-    6,2f\n\t"
	"fadd    1,1,13\n\t"
	"fsub    1,1,13\n\t"
	"fabs    1,1\n\t"
	"mtfsf   0x01,11\n\t"
	"blr\n"
"2:"	"bge-    6,3f\n\t"
	"fsub    1,1,13\n\t"
	"fadd    1,1,13\n\t"
	"fnabs   1,1\n"
"3:"	"mtfsf   0x01,11\n\t"
	"blr\n\t"
	"# frin == round\n"
"frin:"	"mffs    11\n\t"
#ifdef __powerpc64__
	"lfd     13,8b@toc(2)\n\t"
#else
	"mflr    11\n\t"
	"bcl     20,31,1f\n"
"1:"	"mflr    9\n\t"
	"addis   9,9,8b-1b@ha\n\t"
	"addi    9,9,8b-1b@l\n\t"
	"mtlr    11\n\t"
	"lfs     13,0(9)\n\t"
#endif
	"fabs    0,1\n\t"
	"fsub    12,13,13\n\t"
	"fcmpu   7,0,13\n\t"
	"fcmpu   6,1,12\n\t"
	"bnllr-  7\n\t"
	"mtfsfi  7,1\n\t"
#ifdef __powerpc64__
	"lfd     10,9b@toc(2)\n\t"
#else
	"lfs     10,9b-8b(9)\n\t"
#endif
	"ble-    6,2f\n\t"
	"fadd    1,1,10\n\t"
	"fadd    1,1,13\n\t"
	"fsub    1,1,13\n\t"
	"fabs    1,1\n\t"
	"mtfsf   0x01,11\n\t"
	"blr\n"
"2:"	"fsub    9,1,10\n\t"
	"bge-    6,3f\n\t"
	"fsub    1,9,13\n\t"
	"fadd    1,1,13\n\t"
	"fnabs   1,1\n"
"3:"	"mtfsf   0x01,11\n\t"
	"blr\n\t"
	"# frim == floor\n"
"frim:"	"mffs    11\n\t"
#ifdef __powerpc64__
	"lfd     13,8b@toc(2)\n\t"
#else
	"mflr    11\n\t"
	"bcl     20,31,1f\n"
"1:"	"mflr    9\n\t"
	"addis   9,9,8b-1b@ha\n\t"
	"lfs     13,8b-1b@l(9)\n\t"
	"mtlr    11\n\t"
#endif
	"fabs    0,1\n\t"
	"fsub    12,13,13\n\t"
	"fcmpu   7,0,13\n\t"
	"fcmpu   6,1,12\n\t"
	"bnllr-  7\n\t"
	"mtfsfi  7,3\n\t"
	"ble-    6,2f\n\t"
	"fadd    1,1,13\n\t"
	"fsub    1,1,13\n\t"
	"fabs    1,1\n\t"
	"mtfsf   0x01,11\n\t"
	"blr\n"
"2:"	"bge-    6,3f\n\t"
	"fsub    1,1,13\n\t"
	"fadd    1,1,13\n\t"
	"fnabs   1,1\n"
"3:"	"mtfsf   0x01,11\n\t"
	"blr\n");

static void
catch_sigill (int signal, struct sigcontext *ctx)
{
  unsigned int insn = *(unsigned int *) (ctx->regs->nip);
#ifdef __powerpc64__
  if ((insn & 0xfc1f07ff) == 0x7c0005be) /* mftgpr */
    {
      unsigned long *regs = (unsigned long *) ctx->regs;
      unsigned fpr = (insn >> 11) & 0x1f;
      unsigned gpr = (insn >> 21) & 0x1f;
      regs[gpr] = regs[fpr + 0x30];
      ctx->regs->nip += 4;
      return;
    }
  if ((insn & 0xfc1f07ff) == 0x7c0004be) /*mffgpr */
    {
      unsigned long *regs = (unsigned long *) ctx->regs;
      unsigned fpr = (insn >> 21) & 0x1f;
      unsigned gpr = (insn >> 11) & 0x1f;
      regs[fpr + 0x30] = regs[gpr];
      ctx->regs->nip += 4;
      return;
    }
#endif
  if ((insn & 0xfc1f073f) == 0xfc000310) /* fri[pznm] */
    {
#ifdef __powerpc64__
      double *regs = (double *) (((char *) ctx->regs) + 0x30 * 8);
      unsigned int *fpscr = (unsigned int *) (((char *) ctx->regs) + 0x50 * 8 + 4);
#else
      double *regs = (double *) (((char *) ctx->regs) + 0x30 * 4);
      unsigned int *fpscr = (unsigned int *) (((char *) ctx->regs) + 0x30 * 4 + 0x20 * 8 + 4);
#endif
      unsigned dest = (insn >> 21) & 0x1f;
      unsigned src = (insn >> 11) & 0x1f;
      switch (insn & 0xc0)
	{
	case 0:
	  regs[dest] = frin (regs[src]);
	  break;
	case 0x40:
	  regs[dest] = friz (regs[src]);
	  break;
	case 0x80:
	  regs[dest] = frip (regs[src]);
	  break;
	case 0xc0:
	  regs[dest] = frim (regs[src]);
	  break;
	}
      /* Update raised exceptions.  */
      union { unsigned int i[2]; double d; } u;
      asm volatile ("mffs %0" : "=f" (u.d));
      u.i[1] &= 0xfffe0000; /* Is this correct?  */
      *fpscr |= u.i[1];
      ctx->regs->nip += 4;
      return;
    }

  struct sigaction sa;
  sa.sa_handler = SIG_DFL;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction (signal, &sa, NULL);
  raise (signal);
}

static void
__attribute__ ((constructor))
install_handler (void)
{
  struct sigaction sa;
  sa.sa_handler = (void *) catch_sigill;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction (SIGILL, &sa, NULL);
}
