/* Shared definition for glob and glob_pattern_p.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#ifndef GLOB_INTERNAL_H
# define GLOB_INTERNAL_H

static inline int
__glob_pattern_type (const char *pattern, int quote)
{
  const char *p;
  int ret = 0;

  for (p = pattern; *p != '\0'; ++p)
    switch (*p)
      {
      case '?':
      case '*':
        return 1;

      case '\\':
        if (quote)
          {
            if (p[1] != '\0')
              ++p;
            ret |= 2;
          }
        break;

      case '[':
        ret |= 4;
        break;

      case ']':
        if (ret & 4)
          return 1;
        break;
      }

  return ret;
}

#endif /* GLOB_INTERNAL_H  */
