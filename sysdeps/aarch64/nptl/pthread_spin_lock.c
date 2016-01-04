/* Copyright (C) 2008-2016 Free Software Foundation, Inc.

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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define SPIN_LOCK_READS_BETWEEN_CMPXCHG 1000

/* We can't use the normal "#include <nptl/pthread_spin_lock.c>" because
   it will resolve to this very file.  Using "sysdeps/.." as reference to the
   top level directory does the job.  */
#include <sysdeps/../nptl/pthread_spin_lock.c>
