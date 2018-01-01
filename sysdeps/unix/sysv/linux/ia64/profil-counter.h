/* Machine-dependent SIGPROF signal handler.  IA-64 version.
   Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

/* In many Unix systems signal handlers are called like this
   and the interrupted PC is easily findable in the `struct sigcontext'.  */

static void
__profil_counter (int signr, siginfo_t *si, struct sigcontext *scp)
{
  unsigned long ip = scp->sc_ip & ~0X3ULL, slot = scp->sc_ip & 0x3ull;

  /* Note: Linux/ia64 encodes the slot number in bits 0 and 1.  We
     want to multiply the slot number by four so we can use bins of
     width 4 to get accurate instruction-level profiling.  */
  profil_count ((void *) (ip + 4*slot));
}
