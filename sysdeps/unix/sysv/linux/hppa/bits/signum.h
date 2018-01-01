/* Signal number definitions.  Linux/HPPA version.
   Copyright (C) 1995-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _BITS_SIGNUM_H
#define _BITS_SIGNUM_H 1

#ifndef _SIGNAL_H
#error "Never include <bits/signum.h> directly; use <signal.h> instead."
#endif

#include <bits/signum-generic.h>

/* Adjustments and additions to the signal number constants for
   Linux/HPPA.  These values were originally chosen for HP/UX
   compatibility, but were renumbered as of kernel 3.17 and glibc 2.21
   to accommodate software (notably systemd) that assumed at least 29
   real-time signal numbers would be available.  SIGEMT and SIGLOST
   were removed, and the values of SIGSTKFLT, SIGXCPU, XIGXFSZ, and
   SIGSYS were changed, enabling __SIGRTMIN to be 32.  */

#define	SIGSTKFLT	 7	/* Stack fault (obsolete).  */
#define	SIGPWR		19	/* Power failure imminent.  */

#undef	SIGXCPU
#define	SIGXCPU		12
#undef	SIGUSR1
#define	SIGUSR1		16
#undef	SIGUSR2
#define SIGUSR2		17
#undef	SIGCHLD
#define	SIGCHLD		18
#undef	SIGVTALRM
#define	SIGVTALRM	20
#undef	SIGPROF
#define	SIGPROF		21
#undef	SIGPOLL
#define	SIGPOLL		22
#undef	SIGWINCH
#define	SIGWINCH	23
#undef	SIGSTOP
#define	SIGSTOP		24
#undef	SIGTSTP
#define	SIGTSTP		25
#undef	SIGCONT
#define	SIGCONT		26
#undef	SIGTTIN
#define	SIGTTIN		27
#undef	SIGTTOU
#define	SIGTTOU		28
#undef	SIGURG
#define	SIGURG		29
#undef	SIGXFSZ
#define	SIGXFSZ		30
#undef	SIGSYS
#define SIGSYS		31

#undef	__SIGRTMAX
#define __SIGRTMAX	64

#endif	/* <signal.h> included.  */
