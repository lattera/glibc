/* Routines for dealing with '\0' separated environment vectors
   Copyright (C) 1995, 1996, 1998 Free Software Foundation, Inc.
   Written by Miles Bader <miles@gnu.ai.mit.edu>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _ENVZ_H
#define _ENVZ_H	1

#include <features.h>

#include <errno.h>

/* Envz's are argz's too, and should be created etc., using the same
   routines.  */
#include <argz.h>

__BEGIN_DECLS

/* Returns a pointer to the entry in ENVZ for NAME, or 0 if there is none.  */
extern char *envz_entry __P ((__const char *__envz, size_t __envz_len,
			      __const char *__name));

/* Returns a pointer to the value portion of the entry in ENVZ for NAME, or 0
   if there is none.  */
extern char *envz_get __P ((__const char *__envz, size_t __envz_len,
			    __const char *__name));

/* Adds an entry for NAME with value VALUE to ENVZ & ENVZ_LEN.  If an entry
   with the same name already exists in ENVZ, it is removed.  If VALUE is
   NULL, then the new entry will a special null one, for which envz_get will
   return NULL, although envz_entry will still return an entry; this is handy
   because when merging with another envz, the null entry can override an
   entry in the other one.  Null entries can be removed with envz_strip ().  */
extern error_t envz_add __P ((char **__envz, size_t *__envz_len,
			      __const char *__name, __const char *__value));

/* Adds each entry in ENVZ2 to ENVZ & ENVZ_LEN, as if with envz_add().  If
   OVERRIDE is true, then values in ENVZ2 will supersede those with the same
   name in ENV, otherwise not.  */
extern error_t envz_merge __P ((char **__envz, size_t *__envz_len,
				__const char *__envz2, size_t __envz2_len,
				int __override));

/* Remove the entry for NAME from ENVZ & ENVZ_LEN, if any.  */
extern void envz_remove __P ((char **__envz, size_t *__envz_len,
			      __const char *__name));

/* Remove null entries.  */
extern void envz_strip __P ((char **__envz, size_t *__envz_len));

__END_DECLS

#endif /* envz.h */
