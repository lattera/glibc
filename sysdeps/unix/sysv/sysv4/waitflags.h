/* Definitions of flag bits for `waitpid' et al.
   Copyright (C) 1993 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_WAITFLAGS_H

#define	_WAITFLAGS_H	1

/* Bits in the third argument to `waitpid'.  */
#define	WNOHANG		64	/* Don't block waiting.  */
#define	WUNTRACED	4	/* Report status of stopped children.  */

#ifdef __USE_SVID
#define WEXITED		1	/* Look for children that have exited.  */
#define WTRAPPED	2	/* Look for processes that stopped
				   while tracing.  */
#endif

#endif	/* waitflags.h */
