/* Structure describing state saved while handling a signal.  Sparc version.
Copyright (C) 1992 Free Software Foundation, Inc.
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

struct sigcontext
  {
    int sc_onstack;
    sigset_t sc_mask;

#define	SPARC_MAXREGWINDOW 31	/* Maximum usable register windows.  */
    int sc_sp, sc_pc, sc_npc, sc_psr, sc_g1, sc_o0;
    int sc_wbcnt;		/* Number of outstanding windows.  */
    __ptr_t sc_spbuf[SPARC_MAXREGWINDOW]; /* SP's for each window.  */
    int sc_wbuf[SPARC_MAXREGWINDOW][16]; /* Saved register windows.  */
  };

