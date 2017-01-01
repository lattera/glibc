/* Entering namespaces for test case isolation.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
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

#ifndef SUPPORT_NAMESPACE_H
#define SUPPORT_NAMESPACE_H

#include <stdbool.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/* Attempts to become root (or acquire root-like privileges), possibly
   with the help of user namespaces.  Return true if (restricted) root
   privileges could be attained in some way.  Print diagnostics to
   standard output.

   Note that this function generally has to be called before a process
   becomes multi-threaded, otherwise it may fail with insufficient
   privileges on systems which would support this operation for
   single-threaded processes.  */
bool support_become_root (void);

/* Enter a network namespace (and a UTS namespace if possible) and
   configure the loopback interface.  Return true if a network
   namespace could be created.  Print diagnostics to standard output.
   If a network namespace could be created, but networking in it could
   not be configured, terminate the process.  It is recommended to
   call support_become_root before this function so that the process
   has sufficient privileges.  */
bool support_enter_network_namespace (void);

/* Return true if support_enter_network_namespace managed to enter a
   UTS namespace.  */
bool support_in_uts_namespace (void);

__END_DECLS

#endif
