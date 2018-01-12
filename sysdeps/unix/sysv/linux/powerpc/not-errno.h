/* Syscall wrapper that do not set errno.  Linux powerpc version.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

/* __access_noerrno is used during process initialization in elf/dl-tunables.c
   before the TCB is initialized, prohibiting the usage of
   ABORT_TRANSACTION.  */
#undef ABORT_TRANSACTION
#define ABORT_TRANSACTION

#include "sysdeps/unix/sysv/linux/not-errno.h"

/* Recover ABORT_TRANSACTION's previous value, in order to not affect
   other syscalls.  */
#undef ABORT_TRANSACTION
#define ABORT_TRANSACTION ABORT_TRANSACTION_IMPL
