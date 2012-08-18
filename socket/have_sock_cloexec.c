/* Copyright (C) 2008-2012 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <sys/socket.h>
#include <kernel-features.h>

#if defined SOCK_CLOEXEC && !defined __ASSUME_SOCK_CLOEXEC
int __have_sock_cloexec;
#endif

#if defined O_CLOEXEC && !defined __ASSUME_PIPE2
int __have_pipe2;
#endif

#if defined O_CLOEXEC && !defined __ASSUME_DUP3
int __have_dup3;
#endif
