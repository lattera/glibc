/* Handle real-time signal allocation.  NPTL version.
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
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

#include <signal.h>
#include <nptl/pthreadP.h>

/* Up to three special signals might be used privately by libpthread.
   Figure out how many unique ones are actually used.  */

#ifdef SIGCANCEL
# define SIGCANCEL_CONSUMES     1
#else
# define SIGCANCEL_CONSUMES     0
#endif

#if defined SIGTIMER && (!defined SIGCANCEL || SIGTIMER != SIGCANCEL)
# define SIGTIMER_CONSUMES      1
#else
# define SIGTIMER_CONSUMES      0
#endif

#if (defined SIGSETXID \
     && (!defined SIGCANCEL || SIGSETXID != SIGCANCEL) \
     && (!defined SIGTIMER || SIGSETXID != SIGTIMER))
# define SIGSETXID_CONSUMES     1
#else
# define SIGSETXID_CONSUMES     0
#endif

/* This tells the generic code (included below) how many signal
   numbers need to be reserved for libpthread's private uses.  */
#define RESERVED_SIGRT          \
  (SIGCANCEL_CONSUMES + SIGTIMER_CONSUMES + SIGSETXID_CONSUMES)

#include <signal/allocrtsig.c>
