/* Copyright (C) 1993, 1997 Free Software Foundation, Inc.
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

#include <mach.h>

kern_return_t
__mach_get_priv_ports (mach_port_t *host_priv_ptr,
		       mach_port_t *device_master_ptr)
{
  kern_return_t err;
  mach_port_t bootstrap, reply;

  struct
    {
      mach_msg_header_t hdr;
      mach_msg_type_t host_priv_type;
      mach_port_t host_priv;
      mach_msg_type_t dev_master_type;
      mach_port_t dev_master;
    } msg;

  if (err = task_get_bootstrap_port (mach_task_self (), &bootstrap))
    return err;

  /* We cannot simply use a MiG-generated user stub to do this,
     because the return message does not contain a return code datum.  */
  reply = __mach_reply_port ();
  msg.hdr.msgh_bits = MACH_MSGH_BITS (MACH_MSG_TYPE_COPY_SEND,
				      MACH_MSG_TYPE_MAKE_SEND_ONCE);
  msg.hdr.msgh_size = 0;
  msg.hdr.msgh_remote_port = bootstrap;
  msg.hdr.msgh_local_port = reply;
  msg.hdr.msgh_kind = MACH_MSGH_KIND_NORMAL;
  msg.hdr.msgh_id = 999999;
  err = __mach_msg (&msg.hdr,
		    MACH_SEND_MSG|MACH_RCV_MSG|MACH_RCV_TIMEOUT,
		    sizeof (msg.hdr), sizeof (msg), reply,
		    500, MACH_PORT_NULL); /* XXX timeout is arbitrary */
  mach_port_deallocate (mach_task_self (), bootstrap);
  mach_port_deallocate (mach_task_self (), reply);

  if (err == KERN_SUCCESS)
    {
      *host_priv_ptr = msg.host_priv;
      *device_master_ptr = msg.dev_master;
    }

  return err;
}
