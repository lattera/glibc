/* Linux/ARM version of processor capability information handling macros.
   Copyright (C) 2001, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>, 2001.

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

#ifndef _DL_PROCINFO_H
#define _DL_PROCINFO_H	1

#include <ldsodefs.h>

#define _DL_HWCAP_COUNT 8

/* The kernel provides platform data but it is not interesting.  */
#define _DL_HWCAP_PLATFORM 	0


static inline int
__attribute__ ((unused))
_dl_procinfo (int word)
{
  int i;

  _dl_printf ("AT_HWCAP:   ");

  for (i = 0; i < _DL_HWCAP_COUNT; ++i)
    if (word & (1 << i))
      _dl_printf (" %s", GLRO(dl_arm_cap_flags)[i]);

  _dl_printf ("\n");

  return 0;
}

static inline const char *
__attribute__ ((unused))
_dl_hwcap_string (int idx)
{
  return GLRO(dl_arm_cap_flags)[idx];
};

enum
{
  HWCAP_ARM_SWP	      = 1 << 0,
  HWCAP_ARM_HALF      = 1 << 1,
  HWCAP_ARM_THUMB     = 1 << 2,
  HWCAP_ARM_26BIT     = 1 << 3,
  HWCAP_ARM_FAST_MULT = 1 << 4,
  HWCAP_ARM_FPA       = 1 << 5,
  HWCAP_ARM_VFP       = 1 << 6,
  HWCAP_ARM_EDSP      = 1 << 7,

  HWCAP_IMPORTANT = (HWCAP_ARM_HALF | HWCAP_ARM_FAST_MULT)
};

static inline int
__attribute__ ((unused))
_dl_string_hwcap (const char *str)
{
  int i;

  for (i = 0; i < _DL_HWCAP_COUNT; i++)
    {
      if (strcmp (str, GLRO(dl_arm_cap_flags)[i]) == 0)
	return i;
    }
  return -1;
};

#define _dl_string_platform(str) (-1)

#endif /* dl-procinfo.h */
