/* _longjmp_unwind -- Clean up stack frames unwound by longjmp.  Hurd version.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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

#include <setjmp.h>
#include <hurd/userlink.h>
#include <hurd/signal.h>
#include <hurd/sigpreempt.h>
#include <assert.h>


#ifndef _JMPBUF_UNWINDS
 #error "sysdeps/MACHINE/jmp_buf.h fails to define _JMPBUF_UNWINDS"
#endif

/* This function is called by `longjmp' (with its arguments) to restore
   active resources to a sane state before the frames code using them are
   jumped out of.  */

void
_longjmp_unwind (jmp_buf env, int val)
{
  struct hurd_sigstate *ss = _hurd_self_sigstate ();
  struct hurd_userlink *link;

  /* All access to SS->active_resources must take place inside a critical
     section where signal handlers cannot run.  */
  __spin_lock (&ss->lock);
  assert (! ss->critical_section);
  ss->critical_section = 1;

  /* Remove local signal preempters being unwound past.  */
  while (ss->preempters &&
	 _JMPBUF_UNWINDS (env[0].__jmpbuf, ss->preempters))
    ss->preempters = ss->preempters->next;

  __spin_unlock (&ss->lock);

  /* Iterate over the current thread's list of active resources.
     Process the head portion of the list whose links reside
     in stack frames being unwound by this jump.  */

  for (link = ss->active_resources;
       link && _JMPBUF_UNWINDS (env[0].__jmpbuf, link);
       link = link->thread.next)
    /* Remove this link from the resource's users list,
       since the frame using the resource is being unwound.
       This call returns nonzero if that was the last user.  */
    if (_hurd_userlink_unlink (link))
      /* One of the frames being unwound by the longjmp was the last user
	 of its resource.  Call the cleanup function to deallocate it.  */
      (*link->cleanup) (link->cleanup_data, env, val);

  _hurd_critical_section_unlock (ss);
}
