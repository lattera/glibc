/* Copyright (C) 1991, 1992, 1994, 1995 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <signal.h>
#include <errno.h>
#include <stddef.h>


/* We use a wrapper handler to support SV_RESETHAND.  */

static __sighandler_t wrapped_handlers[NSIG];
static sigset_t wrapped_masks[NSIG];

static void
DEFUN(wrapper_handler, (sig), int sig)
{
  int save;
  struct sigaction act;

  act.sa_handler = SIG_DFL;
  act.sa_mask = wrapped_masks[sig];
  act.sa_flags = 0;
  save = errno;
  (void) __sigaction(sig, &act, (struct sigaction *) NULL);
  errno = save;

  (*wrapped_handlers[sig])(sig);
}

static
#ifdef	__GNUC__
inline
#endif
int
DEFUN(convert_mask, (set, mask), sigset_t *set AND CONST int mask)
{
  register int sig;

  if (sizeof(*set) == sizeof(mask))
    {
      *(int *) set = mask;
      return 0;
    }

  if (__sigemptyset(set) < 0)
    return -1;

  for (sig = 1; sig < NSIG; ++sig)
    if (mask & sigmask(sig))
      if (__sigaddset(set, sig) < 0)
	return -1;

  return 0;
}

/* If VEC is non-NULL, set the handler for SIG to the `sv_handler' member
   of VEC.  The signals in `sv_mask' will be blocked while the handler runs.
   If the SV_RESETHAND bit is set in `sv_flags', the handler for SIG will be
   reset to SIG_DFL before `sv_handler' is entered.  If OVEC is non-NULL,
   it is filled in with the old information for SIG.  */
int
DEFUN(__sigvec, (sig, vec, ovec),
      int sig AND CONST struct sigvec *vec AND struct sigvec *ovec)
{
  struct sigaction old;

  if (vec == NULL || !(vec->sv_flags & SV_RESETHAND))
    {
      struct sigaction new, *n;

      if (vec == NULL)
	n = NULL;
      else
	{
	  n = &new;
	  n->sa_handler = vec->sv_handler;
	  if (convert_mask (&n->sa_mask, vec->sv_mask) < 0)
	    return -1;
	  n->sa_flags = 0;
	  
	  if (vec->sv_flags & SV_ONSTACK)
	    {
#ifdef SA_ONSTACK
	      n->sa_flags |= SA_ONSTACK;
#else
	      errno = ENOSYS;
	      return -1;
#endif
	    }
#ifdef SA_RESTART
	  if (!(vec->sv_flags & SV_INTERRUPT))
	    n->sa_flags |= SA_RESTART;
#endif
	}

      if (__sigaction (sig, n, &old) < 0)
	return -1;
    }
  else
    {
      struct sigaction wrapper;

      wrapper.sa_handler = wrapper_handler;
      wrapped_handlers[sig] = vec->sv_handler;
      if (convert_mask (&wrapped_masks[sig], vec->sv_mask) < 0)
	return -1;

      if (__sigaction (sig, &wrapper, &old) < 0)
	return -1;
    }

  if (ovec != NULL)
    {
      register int i;
      int mask = 0;

      if (sizeof (int) == sizeof (sigset_t))
	mask = *(int *) &old.sa_mask;
      else
	for (i = 1; i < NSIG; ++i)
	  if (__sigismember(&old.sa_mask, i))
	    mask |= sigmask(i);

      ovec->sv_mask = mask;
      ovec->sv_flags = 0;
#ifdef SA_ONSTACK
      if (old.sa_flags & SA_ONSTACK)
	ovec->sv_flags |= SV_ONSTACK;
#endif
#ifdef SA_RESTART
      if (!(old.sa_flags & SA_RESTART))
#endif
	ovec->sv_flags |= SV_INTERRUPT;
      if (old.sa_handler == wrapper_handler)
	{
	  ovec->sv_flags |= SV_RESETHAND;
	  ovec->sv_handler = wrapped_handlers[sig];
	}
      else
	ovec->sv_handler = old.sa_handler;
    }

  return 0;
}

weak_alias (__sigvec, sigvec)
