/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef _MCHECK_H
#define _MCHECK_H	1

#include <features.h>

__BEGIN_DECLS

/* Return values for `mprobe': these are the kinds of inconsistencies that
   `mcheck' enables detection of.  */
enum mcheck_status
  {
    MCHECK_DISABLED = -1,       /* Consistency checking is not turned on.  */
    MCHECK_OK,                  /* Block is fine.  */
    MCHECK_FREE,                /* Block freed twice.  */
    MCHECK_HEAD,                /* Memory before the block was clobbered.  */
    MCHECK_TAIL                 /* Memory after the block was clobbered.  */
  };


/* Activate a standard collection of debugging hooks.  This must be called
   before `malloc' is ever called.  ABORTFUNC is called with an error code
   (see enum above) when an inconsistency is detected.  If ABORTFUNC is
   null, the standard function prints on stderr and then calls `abort'.  */
extern int mcheck __P ((void (*__abortfunc) (enum mcheck_status)));

/* Check for aberrations in a particular malloc'd block.  You must have
   called `mcheck' already.  These are the same checks that `mcheck' does
   when you free or reallocate a block.  */
extern enum mcheck_status mprobe __P ((__ptr_t __ptr));

/* Activate a standard collection of tracing hooks.  */
extern void mtrace __P ((void));
extern void muntrace __P ((void));

__END_DECLS

#endif /* mcheck.h */
