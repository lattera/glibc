/* "Hello world" program for GNU C Library on bare Mach 3.0.
   Copyright (C) 1991, 1992, 1993, 1997 Free Software Foundation, Inc.
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
#include <device/device.h>
#include <errno.h>
#include <stdio.h>

int
main (void)
{
  kern_return_t err;
  mach_port_t device, consdev;
  FILE *consf;

  err = get_privileged_ports (NULL, &device);
  if (err)
    _exit (err);
  err = device_open (device, D_WRITE, "console", &consdev);
  mach_port_deallocate (mach_task_self (), device);
  if (err)
    _exit (err);

  consf = mach_open_devstream (consdev, "w");
  if (consf == NULL)
    exit (errno);

  fputs ("Hello, world!\n", consf);

  return 0;
}
