/* Copyright (C) 1994, 1997 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/*  m68020.h
 *
 *  This file contains macros which are used to access MC68020
 *  registers which are not addressable by C.  These are
 *  useful when developing the board specific support.
 */

#ifndef m68020_h__
#define m68020_h__

typedef void ( *mc68020_isr )( void );

#define disable_intr( level ) \
  { (level) = 0; \
    asm volatile ( "movew   %%sr,%0 ; \
		    orw     #0x0700,%%sr" \
		    : "=d" ((level)) : "0" ((level)) ); \
  }

#define enable_intr( level ) \
  { asm volatile ( "movew   %0,%%sr " \
		       : "=d" ((level)) : "0" ((level)) ); \
  }

#define flash_intr( level ) \
  { asm volatile ( "movew   %0,%%sr ; \
		    orw     #0x0700,%%sr" \
		       : "=d" ((level)) : "0" ((level)) ); \
  }

#define get_vbr( vbr ) \
  { (vbr) = 0; \
    asm volatile ( "movec   %%vbr,%0 " \
		       : "=a" (vbr) : "0" (vbr) ); \
  }

#define set_vbr( vbr ) \
  { register mc68020_isr *_vbr= (mc68020_isr *)(vbr); \
    asm volatile ( "movec   %0,%%vbr " \
		       : "=a" (_vbr) : "0" (_vbr) ); \
  }

#define enable_caching() \
  { register unsigned int _ctl=0x01; \
    asm volatile ( "movec   %0,%%cacr" \
		       : "=d" (_ctl) : "0" (_ctl) ); \
  }

#define delay( microseconds ) \
  { register unsigned int _delay=(microseconds); \
    register unsigned int _tmp=123; \
    asm volatile( "0: \
		     nbcd      %0 ; \
		     nbcd      %0 ; \
		     dbf       %1,0 " \
		  : "=d" (_tmp), "=d" (_delay) \
		  : "0"  (_tmp), "1"  (_delay) ); \
  }

#define enable_tracing()
#define cause_intr( X )
#define clear_intr( X )

extern mc68020_isr     M68Kvec[];   /* vector table address */

#endif
/* end of include file */
