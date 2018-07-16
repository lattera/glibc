/* Additional fields in struct link_map.  Linux/x86 version.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

/* If this object is enabled with CET.  */
enum
  {
    lc_none = 0,			 /* Not enabled with CET.  */
    lc_ibt = 1 << 0,			 /* Enabled with IBT.  */
    lc_shstk = 1 << 1,			 /* Enabled with STSHK.  */
    lc_ibt_and_shstk = lc_ibt | lc_shstk /* Enabled with both.  */
  } l_cet:2;
