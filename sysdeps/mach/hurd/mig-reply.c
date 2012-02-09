/* Copyright (C) 1994,1995,1996,1997,2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <mach.h>
#include <hurd/threadvar.h>

#define GETPORT \
  mach_port_t *portloc = \
    (mach_port_t *) __hurd_threadvar_location (_HURD_THREADVAR_MIG_REPLY)
#define reply_port (*(use_threadvar ? portloc : &global_reply_port))

static int use_threadvar;
static mach_port_t global_reply_port;

/* These functions are called by MiG-generated code.  */

/* Called by MiG to get a reply port.  */
mach_port_t
__mig_get_reply_port (void)
{
  GETPORT;

  if (reply_port == MACH_PORT_NULL)
    reply_port = __mach_reply_port ();

  return reply_port;
}
weak_alias (__mig_get_reply_port, mig_get_reply_port)

/* Called by MiG to deallocate the reply port.  */
void
__mig_dealloc_reply_port (mach_port_t arg)
{
  mach_port_t port;

  GETPORT;

  port = reply_port;
  reply_port = MACH_PORT_NULL;	/* So the mod_refs RPC won't use it.  */

  if (MACH_PORT_VALID (port))
    __mach_port_mod_refs (__mach_task_self (), port,
			  MACH_PORT_RIGHT_RECEIVE, -1);
}
weak_alias (__mig_dealloc_reply_port, mig_dealloc_reply_port)

/* Called by mig interfaces when done with a port.  Used to provide the
   same interface as needed when a custom allocator is used.  */
void
__mig_put_reply_port(mach_port_t port)
{
  /* Do nothing.  */
}
weak_alias (__mig_put_reply_port, mig_put_reply_port)

/* Called at startup with STACK == NULL.  When per-thread variables are set
   up, this is called again with STACK set to the new stack being switched
   to, where per-thread variables should be set up.  */
void
__mig_init (void *stack)
{
  use_threadvar = stack != 0;

  if (use_threadvar)
    {
      /* Recycle the reply port used before multithreading was enabled.  */
      mach_port_t *portloc = (mach_port_t *)
	__hurd_threadvar_location_from_sp (_HURD_THREADVAR_MIG_REPLY, stack);
      *portloc = global_reply_port;
      global_reply_port = MACH_PORT_NULL;
    }
}
weak_alias (__mig_init, mig_init)
