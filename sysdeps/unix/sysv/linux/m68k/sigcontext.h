/* Structure describing state saved while handling a signal.  Linux/m68k version.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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

/* State of this thread when the signal was taken.  */
struct sigcontext
{
  __sigset_t sc_mask;
  unsigned long sc_usp;
  unsigned long sc_d0;
  unsigned long sc_d1;
  unsigned long sc_a0;
  unsigned long sc_a1;
  unsigned short sc_sr;
  unsigned long sc_pc;
};
