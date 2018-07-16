/* x86 CET tuning.
   This file is part of the GNU C Library.
   Copyright (C) 2018 Free Software Foundation, Inc.

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

/* Valid control values:
   0: Enable CET features based on ELF property note.
   1: Always disable CET features.
   2: Always enable CET features.
   3: Enable CET features permissively.
 */
#define CET_ELF_PROPERTY	0
#define CET_ALWAYS_OFF		1
#define CET_ALWAYS_ON		2
#define CET_PERMISSIVE		3
#define CET_MAX			CET_PERMISSIVE
