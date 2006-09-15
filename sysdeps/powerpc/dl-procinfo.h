/* Processor capability information handling macros.  PowerPC version.
   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

#ifndef _DL_PROCINFO_H
#define _DL_PROCINFO_H	1

#include <ldsodefs.h>
#include <sysdep.h>		/* This defines the PPC_FEATURE_* macros.  */

/* There are 20 bits used, but they are bits 12..31.  */
#define _DL_HWCAP_FIRST		12
#define _DL_HWCAP_COUNT		32

/* These bits influence library search.  */
#define HWCAP_IMPORTANT		(PPC_FEATURE_HAS_ALTIVEC)

#define _DL_PLATFORMS_COUNT	6

#define _DL_FIRST_PLATFORM      32
/* Mask to filter out platforms.  */
#define _DL_HWCAP_PLATFORM      (((1ULL << _DL_PLATFORMS_COUNT) - 1) \
                                 << _DL_FIRST_PLATFORM)

static inline const char *
__attribute__ ((unused))
_dl_hwcap_string (int idx)
{
  return GLRO(dl_powerpc_cap_flags)[idx - _DL_HWCAP_FIRST];
}

static inline const char *
__attribute__ ((unused))
_dl_platform_string (int idx)
{
  return GLRO(dl_powerpc_platforms)[idx - _DL_FIRST_PLATFORM];
}

static inline int
__attribute__ ((unused))
_dl_string_hwcap (const char *str)
{
  for (int i = _DL_HWCAP_FIRST; i < _DL_HWCAP_COUNT; ++i)
    if (strcmp (str, _dl_hwcap_string (i)) == 0)
      return i;
  return -1;
}

static inline int
__attribute__ ((unused, always_inline))
_dl_string_platform (const char *str)
{
  if (str == NULL)
    return -1;

  if (strncmp (str, GLRO(dl_powerpc_platforms)[0], 5) == 0)
    {
      int ret;
      str += 5;
      switch (*str)
	{
	case '4':
	  ret = _DL_FIRST_PLATFORM + 0;
	  break;
	case '5':
	  ret = _DL_FIRST_PLATFORM + 2;
	  if (str[1] == '+')
	    ++ret, ++str;
	  break;
	case '6':
	  ret = _DL_FIRST_PLATFORM + 4;
	  break;
	default:
	  return -1;
	}
      if (str[1] == '\0')
	return ret;
    }
  else if (strncmp (str, GLRO(dl_powerpc_platforms)[1], 3) == 0)
    {
      if (strcmp (str + 3, GLRO(dl_powerpc_platforms)[1] + 3) == 0)
	return _DL_FIRST_PLATFORM + 1;
      else if (strcmp (str + 3, GLRO(dl_powerpc_platforms)[5] + 3) == 0)
	return _DL_FIRST_PLATFORM + 5;
    }

  return -1;
}

#ifdef IS_IN_rtld
static inline int
__attribute__ ((unused))
_dl_procinfo (int word)
{
  _dl_printf ("AT_HWCAP:       ");

  for (int i = _DL_HWCAP_FIRST; i < _DL_HWCAP_COUNT; ++i)
    if (word & (1 << i))
      _dl_printf (" %s", _dl_hwcap_string (i));

  _dl_printf ("\n");

  return 0;
}
#endif

#endif /* dl-procinfo.h */
