/* wcscpy.c - Wide Character Copy for powerpc32/power6.
   Copyright (C) 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <stddef.h>
#include <wchar.h>


/* Copy SRC to DEST.  */
wchar_t *
wcscpy (dest, src)
     wchar_t *dest;
     const wchar_t *src;
{
  wint_t c,d;
  wchar_t *wcp, *wcp2;

  if (__alignof__ (wchar_t) >= sizeof (wchar_t))
    {
      const ptrdiff_t off = dest - src;

      wcp = (wchar_t *) src;
      wcp2 = wcp + 1 ;

      do
        {
          d = *wcp;
          wcp[off] = d;
          if (d == L'\0')
            return dest;
          wcp += 2;

          c = *wcp2;
          wcp2[off] = c;
          if (c == L'\0')
            return dest;
          wcp2 += 2;

          d = *wcp;
          wcp[off] = d;
          if (d == L'\0')
            return dest;
          wcp += 2;

          c = *wcp2;
          wcp2[off] = c;
          if (c == L'\0')
            return dest;
          wcp2 += 2;

          d = *wcp;
          wcp[off] = d;
          if (d == L'\0')
            return dest;
          wcp += 2;

          c = *wcp2;
          wcp2[off] = c;
          if (c == L'\0')
            return dest;
          wcp2 += 2;

          d = *wcp;
          wcp[off] = d;
          if (d == L'\0')
            return dest;
          wcp += 2;

          c = *wcp2;
          wcp2[off] = c;
          if (c == L'\0')
            return dest;
          wcp2 += 2;
        }
      while (c != L'\0');

    }
  else
    {
      wcp = dest;

      do
        {
          c = *src++;
          *wcp++ = c;
        }
      while (c != L'\0');
    }
  return dest;
}
