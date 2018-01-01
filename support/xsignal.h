/* Support functionality for using signals.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

#ifndef SUPPORT_SIGNAL_H
#define SUPPORT_SIGNAL_H

#include <signal.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/* The following functions call the corresponding libc functions and
   terminate the process on error.  */

void xraise (int sig);
sighandler_t xsignal (int sig, sighandler_t handler);
void xsigaction (int sig, const struct sigaction *newact,
                 struct sigaction *oldact);

/* The following functions call the corresponding libpthread functions
   and terminate the process on error.  */

void xpthread_sigmask (int how, const sigset_t *set, sigset_t *oldset);

__END_DECLS

#endif /* SUPPORT_SIGNAL_H */
