/* Copyright (C) 1994 Free Software Foundation, Inc.
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

static error_t
getbootstrap (mach_port_t *result)
{
  return __task_get_special_port (__mach_task_self (),
				  TASK_BOOTSTRAP_PORT,
				  result);
}

error_t (*_hurd_ports_getters[INIT_PORT_MAX]) (mach_port_t *result) =
  {
    [INIT_PORT_BOOTSTRAP] = getbootstrap,
  };

error_t
_hurd_ports_get (int which, mach_port_t *result)
{
  if (which < 0 || which >= _hurd_nports)
    return EINVAL;
  if (which >= INIT_PORT_MAX || _hurd_ports_getters[which] == NULL)
    return HURD_PORT_USE (&_hurd_ports[which],
			  __mach_port_mod_refs (__mach_task_self (),
						(*result = port),
						MACH_PORT_RIGHT_SEND,
						+1));
  return (*_hurd_ports_getters[which]) (result);
}
