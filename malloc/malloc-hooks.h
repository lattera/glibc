/* Internal declarations of malloc hooks no longer in the public API.
   Copyright (C) 2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _MALLOC_HOOKS_H
#define _MALLOC_HOOKS_H

/* These hooks are no longer part of the public API and are poisoned
   in <stdc-predef.h>.  Their names here reflect the command-line
   mapping which is used inside glibc to get past the poisoning.  */
void (*old__malloc_initialize_hook) (void);

#endif  /* _MALLOC_HOOKS_H */
