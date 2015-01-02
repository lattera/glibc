/* Low-level statistical profiling support function.  Linux/SH version.
   Copyright (C) 1996-2015 Free Software Foundation, Inc.
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

static void
__profil_counter (int signo, int _a2, int _a3, int _a4, struct sigcontext sc)
{
  void *pc;
  pc = (void *) sc.sc_pc;
  profil_count (pc);

  /* This is a hack to prevent the compiler from implementing the
     above function call as a sibcall.  The sibcall would overwrite
     the signal context.  */
  asm volatile ("");
}
