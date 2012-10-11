/* Copyright (C) 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <sched.h>
#include <sysdep.h>
#include <arch/spr_def.h>


/* The count of cores horizontally (X dimension) on the chip.  */
static int chip_width;

/* Read the chip "width" from the /sys filesystem.  */
static int
initialize_chip_width (void)
{
  int w = 0;
  int fd;

  fd = __open ("/sys/devices/system/cpu/chip_width", O_RDONLY);
  if (fd >= 0)
    {
      char buf[64];
      ssize_t n;
      int i;

      n = __read (fd, buf, sizeof (buf));
      __close (fd);

      for (i = 0; i < n; ++i)
        {
          if (buf[i] < '0' || buf[i] > '9')
            break;
          w = (w * 10) + (buf[i] - '0');
        }
    }

  /* Store a negative value so we don't try again.  */
  if (w == 0)
    w = -1;

  /* Using an atomic idempotent write here makes this thread-safe.  */
  chip_width = w;
  return w;
}

int
sched_getcpu (void)
{
  unsigned int coord;
  int w = chip_width;

  if (__builtin_expect (w <= 0, 0))
    {
      if (w == 0)
        w = initialize_chip_width ();
      if (w < 0)
        {
          unsigned int cpu;
          int r = INLINE_SYSCALL (getcpu, 3, &cpu, NULL, NULL);
          return r == -1 ? r : cpu;
        }
    }

  /* Assign 64-bit value to a 32-bit variable to ensure 32-bit multiply.  */
  coord = __insn_mfspr (SPR_TILE_COORD);

  /* Extract Y coord from bits 7..10 and X coord from bits 18..21.  */
  return ((coord >> 7) & 0xf) * w + ((coord >> 18) & 0xf);
}
