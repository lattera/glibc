/* Copyright (C) 1991,1992,1994,1996,1997,2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

/* Get the definition of `struct sigcontext'.  */
#define	KERNEL
#define	sigvec		sun_sigvec
#define	sigstack	sun_sigstack
#define	sigcontext	sun_sigcontext
#include "/usr/include/sys/signal.h"
#undef	sigvec
#undef	sigstack
#undef	sigcontext
#undef	NSIG
#undef	SIGABRT
#undef	SIGCLD
#undef	SV_ONSTACK
#undef	SV_RESETHAND
#undef	SV_INTERRUPT
#undef	SA_ONSTACK
#undef	SA_NOCLDSTOP
#undef	SIG_ERR
#undef	SIG_DFL
#undef	SIG_IGN
#undef	sigmask
#undef	SIG_BLOCK
#undef	SIG_UNBLOCK
#undef	SIG_SETMASK

#include <signal.h>
#include <stddef.h>
#include <errno.h>

/* Defined in __sigvec.S.  */
extern int __raw_sigvec (int sig, CONST struct sigvec *vec,
			 struct sigvec *ovec);

/* User-specified signal handlers.  */
#define mytramp 1
#ifdef mytramp
static __sighandler_t handlers[NSIG];
#else
#define handlers _sigfunc
extern __sighandler_t _sigfunc[];
#endif

#if mytramp

/* Handler for all signals that are handled by a user-specified function.
   Saves and restores the general regs %g2-%g7, the %y register, and
   all the FPU regs (including %fsr), around calling the user's handler.  */
static void
trampoline (sig)
     int sig;
{
  /* We use `double' and `long long int' so `std' (store doubleword) insns,
     which might be faster than single-word stores, will be generated.  */
  register double f0 asm("%f0");
  register double f2 asm("%f2");
  register double f4 asm("%f4");
  register double f6 asm("%f6");
  register double f8 asm("%f8");
  register double f10 asm("%f10");
  register double f12 asm("%f12");
  register double f14 asm("%f14");
  register double f16 asm("%f16");
  register double f18 asm("%f18");
  register double f20 asm("%f20");
  register double f22 asm("%f22");
  register double f24 asm("%f24");
  register double f26 asm("%f26");
  register double f28 asm("%f28");
  register double f30 asm("%f30");
  register long long int g2 asm("%g2");
  register long long int g4 asm("%g4");
  register long long int g6 asm("%g6");
  register int *fp asm("%fp");

  int code;
  register struct sigcontext *context asm("%i0"); /* See end of fn.  */
  void *addr;
  int y;
  double fpsave[16];
  int fsr;
  int savefpu;
  long long int glsave[3];

  /* SIG isn't really passed as an arg.
     The args to the signal handler are at fp[16..19].  */
  sig = fp[16];
  code = fp[17];
  context = (struct sigcontext *) fp[18];
  addr = (PTR) fp[19];

  /* Save the Y register.  */
  asm("rd %%y, %0" : "=r" (y));

  /* Save the FPU regs if the FPU enable bit is set in the PSR,
     and the signal isn't an FP exception.  */
  savefpu = (context->sc_psr & 0x1000) && sig != SIGFPE;
  if (savefpu)
    {
      fpsave[0] = f0;
      fpsave[1] = f2;
      fpsave[2] = f4;
      fpsave[3] = f6;
      fpsave[4] = f8;
      fpsave[5] = f10;
      fpsave[6] = f12;
      fpsave[7] = f14;
      fpsave[8] = f16;
      fpsave[9] = f18;
      fpsave[10] = f20;
      fpsave[11] = f22;
      fpsave[12] = f24;
      fpsave[13] = f26;
      fpsave[14] = f28;
      fpsave[15] = f30;

      /* Force it into a stack slot so the asm won't barf.  Sigh.  */
      (void) &fsr;
      asm("st %%fsr, %0" : "=m" (fsr));
    }

  /* Save the global registers (except for %g1, which is a scratch reg).  */
  glsave[0] = g2;
  glsave[1] = g4;
  glsave[2] = g6;

  /* Call the user's handler.  */
  (*((void (*) (int sig, int code, struct sigcontext *context,
		void *addr)) handlers[sig]))
    (sig, code, context, addr);

  /* Restore the Y register.  */
  asm("mov %0, %%y" : : "r" (y));

  if (savefpu)
    {
      /* Restore the FPU regs.  */
      f0 = fpsave[0];
      f2 = fpsave[1];
      f4 = fpsave[2];
      f6 = fpsave[3];
      f8 = fpsave[4];
      f10 = fpsave[5];
      f12 = fpsave[6];
      f14 = fpsave[7];
      f16 = fpsave[8];
      f18 = fpsave[9];
      f20 = fpsave[10];
      f22 = fpsave[11];
      f24 = fpsave[12];
      f26 = fpsave[13];
      f28 = fpsave[14];
      f30 = fpsave[15];

      asm("ld %0, %%fsr" : : "m" (fsr));
    }

  /* Restore the globals.  */
  g2 = glsave[0];
  g4 = glsave[1];
  g6 = glsave[2];

  /* Unwind a frame, and do a "sigcleanup" system call.
     The system call apparently does a return.
     I don't know what it's for.  Ask Sun.  */
  asm("restore %%g0, 139, %%g1\n"
      "ta 0\n"
      "! this should be i0: %0"	/* Useless insn that will never be executed, */
				/* here to make the compiler happy.  */
      : /* No outputs.  */ :
      /* CONTEXT is bound to %i0.  We reference it as an input here to make
	 sure the compiler considers it live at this point, and preserves
	 the value in that register.  The restore makes %i0 become %o0, the
	 argument to the system call.  */
      "r" (context));
}
#endif

int
__sigvec (sig, vec, ovec)
     int sig;
     const struct sigvec *vec;
     struct sigvec *ovec;
{
#ifndef	mytramp
  extern void _sigtramp (int);
#define	trampoline	_sigtramp
#endif
  struct sigvec myvec;
  int mask;
  __sighandler_t ohandler;

  if (sig <= 0 || sig >= NSIG)
    {
      __set_errno (EINVAL);
      return -1;
    }

  mask = __sigblock (sigmask(sig));

  ohandler = handlers[sig];

  if (vec != NULL &&
      vec->sv_handler != SIG_IGN && vec->sv_handler != SIG_DFL)
    {
      handlers[sig] = vec->sv_handler;
      myvec = *vec;
      myvec.sv_handler = trampoline;
      vec = &myvec;
    }

  if (__raw_sigvec(sig, vec, ovec) < 0)
    {
      int save = errno;
      (void) __sigsetmask(mask);
      errno = save;
      return -1;
    }

  if (ovec != NULL && ovec->sv_handler == trampoline)
    ovec->sv_handler = ohandler;

  (void) __sigsetmask (mask);

  return 0;
}
