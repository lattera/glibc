/* Convenience function to catch expected signals during an operation.
Copyright (C) 1996 Free Software Foundation, Inc.
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

#include <hurd/signal.h>
#include <hurd/sigpreempt.h>
#include <string.h>
#include <assert.h>

error_t
hurd_catch_signal (sigset_t sigset,
		   unsigned long int first, unsigned long int last,
		   error_t (*operate) (struct hurd_signal_preempter *),
		   sighandler_t handler)
{
  jmp_buf buf;
  void throw (int signo, long int sigcode, struct sigcontext *scp)
    { longjmp (buf, scp->sc_error ?: EGRATUITOUS); }

  struct hurd_signal_preempter preempter =
    {
      sigset, first, last,
      NULL, handler == SIG_ERR ? (sighandler_t) &throw : handler,
    };

  struct hurd_sigstate *const ss = _hurd_self_sigstate ();
  error_t error;

  if (handler == SIG_ERR)
    /* Not our handler; don't bother saving state.  */
    error = 0;
  else
    /* This returns again with nonzero value when we preempt a signal.  */
    error = setjmp (buf);

  if (error == 0)
    {
      /* Install a signal preempter for the thread.  */
      __spin_lock (&ss->lock);
      preempter.next = ss->preempters;
      ss->preempters = &preempter;
      __spin_unlock (&ss->lock);

      /* Try the operation that might crash.  */
      (*operate) (&preempter);
    }

  /* Either FUNCTION completed happily and ERROR is still zero, or it hit
     an expected signal and `throw' made setjmp return the signal error
     code in ERROR.  Now we can remove the preempter and return.  */

  __spin_lock (&ss->lock);
  assert (ss->preempters == &preempter);
  ss->preempters = preempter.next;
  __spin_unlock (&ss->lock);

  return error;
}


error_t
hurd_safe_memset (void *dest, int byte, size_t nbytes)
{
  error_t operate (struct hurd_signal_preempter *preempter)
    {
      memset (dest, byte, nbytes);
      return 0;
    }
  return hurd_catch_signal (sigmask (SIGBUS) | sigmask (SIGSEGV),
			    (vm_address_t) dest, (vm_address_t) dest + nbytes,
			    &operate, SIG_ERR);
}


error_t
hurd_safe_copyout (void *dest, const void *src, size_t nbytes)
{
  error_t operate (struct hurd_signal_preempter *preempter)
    {
      memcpy (dest, src, nbytes);
      return 0;
    }
  return hurd_catch_signal (sigmask (SIGBUS) | sigmask (SIGSEGV),
			    (vm_address_t) dest, (vm_address_t) dest + nbytes,
			    &operate, SIG_ERR);
}

error_t
hurd_safe_copyin (void *dest, const void *src, size_t nbytes)
{
  error_t operate (struct hurd_signal_preempter *preempter)
    {
      memcpy (dest, src, nbytes);
      return 0;
    }
  return hurd_catch_signal (sigmask (SIGBUS) | sigmask (SIGSEGV),
			    (vm_address_t) src, (vm_address_t) src + nbytes,
			    &operate, SIG_ERR);
}

error_t
hurd_safe_memmove (void *dest, const void *src, size_t nbytes)
{
  jmp_buf buf;
  void throw (int signo, long int sigcode, struct sigcontext *scp)
    { longjmp (buf, scp->sc_error ?: EGRATUITOUS); }

  struct hurd_signal_preempter src_preempter =
    {
      sigmask (SIGBUS) | sigmask (SIGSEGV),
      (vm_address_t) src, (vm_address_t) src + nbytes,
      NULL, (sighandler_t) &throw,
    };
  struct hurd_signal_preempter dest_preempter =
    {
      sigmask (SIGBUS) | sigmask (SIGSEGV),
      (vm_address_t) dest, (vm_address_t) dest + nbytes,
      NULL, (sighandler_t) &throw,
      &src_preempter
    };

  struct hurd_sigstate *const ss = _hurd_self_sigstate ();
  error_t error;

  /* This returns again with nonzero value when we preempt a signal.  */
  error = setjmp (buf);

  if (error == 0)
    {
      /* Install a signal preempter for the thread.  */
      __spin_lock (&ss->lock);
      src_preempter.next = ss->preempters;
      ss->preempters = &dest_preempter;
      __spin_unlock (&ss->lock);

      /* Do the copy; it might fault.  */
      memmove (dest, src, nbytes);
    }

  /* Either memmove completed happily and ERROR is still zero, or it hit
     an expected signal and `throw' made setjmp return the signal error
     code in ERROR.  Now we can remove the preempter and return.  */

  __spin_lock (&ss->lock);
  assert (ss->preempters == &dest_preempter);
  ss->preempters = src_preempter.next;
  __spin_unlock (&ss->lock);

  return error;
}

