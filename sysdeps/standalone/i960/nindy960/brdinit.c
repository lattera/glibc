/* Copyright (C) 1994, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <standalone.h>
#include "i960ca.h"

/*  _Board_Initialize()

This routine initializes the board.

NOTE: Only tested on a Cyclone CVME961 but should be OK on any i960ca board. */

void
_Board_Initialize ()
{
  struct i80960ca_prcb   *prcb;     /* ptr to processor control block */
  struct i80960ca_ctltbl *ctl_tbl;  /* ptr to control table */

  static inline struct i80960ca_prcb *get_prcb()
  { register struct i80960ca_prcb *_prcb = 0;
    asm volatile( "calls 5; \
                   mov   g0,%0" \
                   : "=d" (_prcb) \
                   : "0" (_prcb) );
    return ( _prcb );
  }

  prcb    = get_prcb ();
  ctl_tbl = prcb->control_tbl;

  /*   The following configures the data breakpoint (which must be set
   *   before this is executed) to break on writes only.
   */

  ctl_tbl->bpcon &= ~0x00cc0000;
  reload_ctl_group (6);

   /*  bit 31 of the Register Cache Control can be set to
    *  enable an alternative caching algorithm.  It does
    *  not appear to help our applications.
    */

   /* Configure Number of Register Caches */

  prcb->reg_cache_cfg = 8;
  soft_reset (prcb);
}
