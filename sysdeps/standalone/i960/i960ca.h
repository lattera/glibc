/* Copyright (C) 1994, 1996, 1997 Free Software Foundation, Inc.
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

/* i960ca.h
 *
 *  This file contains macros which are used to access i80960CA
 *  registers which are not addressable by C.  The functions
 *  in this file should be useful to the developer of target
 *  specific code.
 */

#ifndef i960ca_h__
#define i960ca_h__

typedef unsigned char   unsigned8;
typedef unsigned short  unsigned16;
typedef unsigned int    unsigned32;

/*
 *  Intel i80960CA Processor Control Block
 */

struct i80960ca_prcb {
  unsigned32          *fault_tbl;     /* fault table base address     */
  struct i80960ca_ctltbl
                      *control_tbl;   /* control table base address   */
  unsigned32           initial_ac;    /* AC register initial value    */
  unsigned32           fault_config;  /* fault configuration word     */
  void                *intr_tbl;      /* interrupt table base address */
  void                *sys_proc_tbl;  /* system procedure table       */
                                      /*   base address               */
  unsigned32           reserved;      /* reserved                     */
  unsigned32          *intr_stack;    /* interrupt stack pointer      */
  unsigned32           ins_cache_cfg; /* instruction cache            */
                                      /*   configuration word         */
  unsigned32           reg_cache_cfg; /* register cache               */
                                      /*   configuration word         */
};

/*
 *  Intel i80960CA Control Table
 */

struct i80960ca_ctltbl {
                            /* Control Group 0 */
  unsigned32       ipb0;              /* IP breakpoint 0 */
  unsigned32       ipb1;              /* IP breakpoint 1 */
  unsigned32       dab0;              /* data address breakpoint 0 */
  unsigned32       dab1;              /* data address breakpoint 1 */
                            /* Control Group 1 */
  unsigned32       imap0;             /* interrupt map 0 */
  unsigned32       imap1;             /* interrupt map 1 */
  unsigned32       imap2;             /* interrupt map 2 */
  unsigned32       icon;              /* interrupt control */
                            /* Control Group 2 */
  unsigned32       mcon0;             /* memory region 0 configuration */
  unsigned32       mcon1;             /* memory region 1 configuration */
  unsigned32       mcon2;             /* memory region 2 configuration */
  unsigned32       mcon3;             /* memory region 3 configuration */
                            /* Control Group 3 */
  unsigned32       mcon4;             /* memory region 4 configuration */
  unsigned32       mcon5;             /* memory region 5 configuration */
  unsigned32       mcon6;             /* memory region 6 configuration */
  unsigned32       mcon7;             /* memory region 7 configuration */
                            /* Control Group 4 */
  unsigned32       mcon8;             /* memory region 8 configuration */
  unsigned32       mcon9;             /* memory region 9 configuration */
  unsigned32       mcon10;            /* memory region 10 configuration */
  unsigned32       mcon11;            /* memory region 11 configuration */
                            /* Control Group 5 */
  unsigned32       mcon12;            /* memory region 12 configuration */
  unsigned32       mcon13;            /* memory region 13 configuration */
  unsigned32       mcon14;            /* memory region 14 configuration */
  unsigned32       mcon15;            /* memory region 15 configuration */
                            /* Control Group 6 */
  unsigned32       bpcon;             /* breakpoint control */
  unsigned32       tc;                /* trace control */
  unsigned32       bcon;              /* bus configuration control */
  unsigned32       reserved;          /* reserved */
};

#define disable_intr( oldlevel ) \
  { (oldlevel) = 0x1f0000; \
    asm volatile ( "modpc   0,%1,%1" \
                       : "=d" ((oldlevel)) \
                       : "0"  ((oldlevel)) ); \
  }

#define enable_intr( oldlevel ) \
  { unsigned32 _mask = 0x1f0000; \
    asm volatile ( "modpc   0,%0,%1" \
                       : "=d" (_mask), "=d" ((oldlevel)) \
                       : "0"  (_mask), "1"  ((oldlevel)) ); \
  }

#define flash_intr( oldlevel ) \
  { unsigned32 _mask = 0x1f0000; \
    asm volatile ( "modpc   0,%0,%1 ; \
                    mov     %0,%1 ; \
                    modpc   0,%0,%1"  \
                       : "=d" (_mask), "=d" ((oldlevel)) \
                       : "0"  (_mask), "1"  ((oldlevel)) ); \
  }

#define atomic_modify( mask, addr, prev ) \
 { register unsigned32  _mask = (mask); \
   register unsigned32 *_addr = (unsigned32 *)(addr); \
   asm volatile( "atmod  %0,%1,%1" \
                  : "=d" (_addr), "=d" (_mask) \
                  : "0"  (_addr), "1"  (_mask) ); \
   (prev) = _mask; \
 }

#define delay( microseconds ) \
  { register unsigned32 _delay=(microseconds); \
    register unsigned32 _tmp; \
    asm volatile( "delay0: \
                     remo      3,31,%0 ; \
                     cmpo      0,%0 ; \
                     subo      1,%1,%1 ; \
                     cmpobne.t 0,%1,delay0 " \
                  : "=d" (_tmp), "=d" (_delay) \
                  : "0"  (_tmp), "1"  (_delay) ); \
  }

#define enable_tracing() \
 { register unsigned32 _pc = 0x1; \
   asm volatile( "modpc 0,%0,%0" : "=d" (_pc) : "0" (_pc) ); \
 }

#define unmask_intr( xint ) \
 { register unsigned32 _mask= (1<<(xint)); \
   asm volatile( "or sf1,%0,sf1" : "=d" (_mask) : "0" (_mask) ); \
 }

#define mask_intr( xint ) \
 { register unsigned32 _mask= (1<<(xint)); \
   asm volatile( "andnot %0,sf1,sf1" : "=d" (_mask) : "0" (_mask) ); \
 }

#define clear_intr( xint ) \
 { register unsigned32 _xint=(xint); \
   asm volatile( "loop_til_cleared:" \
                 "  clrbit %0,sf0,sf0 ;" \
                 "  bbs    %0,sf0,loop_til_cleared" \
                 : "=d" (_xint) : "0" (_xint) ); \
 }

#define reload_ctl_group( group ) \
 { register int _cmd = ((group)|0x400) ; \
   asm volatile( "sysctl %0,%0,%0" : "=d" (_cmd) : "0" (_cmd) ); \
 }

#define cause_intr( intr ) \
 { register int _intr = (intr); \
   asm volatile( "sysctl %0,%0,%0" : "=d" (_intr) : "0" (_intr) ); \
 }

#define soft_reset( prcb ) \
 { register struct i80960ca_prcb *_prcb = (prcb); \
   register unsigned32         *_next=0; \
   register unsigned32          _cmd  = 0x30000; \
   asm volatile( "lda    next,%1; \
                  sysctl %0,%1,%2; \
            next: mov    g0,g0" \
                  : "=d" (_cmd), "=d" (_next), "=d" (_prcb) \
                  : "0"  (_cmd), "1"  (_next), "2"  (_prcb) ); \
 }

static inline unsigned32 pend_intrs()
{ register unsigned32 _intr=0;
  asm volatile( "mov sf0,%0" : "=d" (_intr) : "0" (_intr) );
  return ( _intr );
}

static inline unsigned32 mask_intrs()
{ register unsigned32 _intr=0;
  asm volatile( "mov sf1,%0" : "=d" (_intr) : "0" (_intr) );
  return( _intr );
}

static inline unsigned32 get_fp()
{ register unsigned32 _fp=0;
  asm volatile( "mov fp,%0" : "=d" (_fp) : "0" (_fp) );
  return ( _fp );
}

#endif
/* end of include file */
