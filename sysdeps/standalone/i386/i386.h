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

/*  i386.h
 *
 *  This file contains macros which are used to access i80386
 *  registers which are not addressable by C.  This file contains
 *  functions which are useful to those developing target
 *  specific support routines.
 */

#ifndef i386_h__
#define i386_h__

typedef unsigned char   unsigned8;
typedef unsigned short  unsigned16;
typedef unsigned int    unsigned32;

#define disable_intr( isrlevel ) \
  { (isrlevel) = 0; \
    asm volatile ( "pushf ; \
		    pop  %0 ; \
		    cli   " \
		    : "=r" ((isrlevel)) : "0" ((isrlevel)) ); \
  }


#define enable_intr( isrlevel ) \
  { asm volatile ( "push %0 ; \
		    popf " \
		    : "=r" ((isrlevel)) : "0" ((isrlevel)) ); \
  }

#define delay( _microseconds ) \
  { \
    unsigned32 _counter; \
    \
    _counter = (_microseconds); \
    \
    asm volatile ( "0: nop;" \
		   " mov %0,%0 ;" \
		   " loop 0" : "=c" (_counter) \
				      : "0"  (_counter) \
		 ); \
    \
  }

/* segment access functions */

static inline unsigned16 get_cs()
{
  register unsigned16 segment = 0;

  asm volatile ( "movw %%cs,%0" : "=r" (segment) : "0" (segment) );

  return segment;
}

static inline unsigned16 get_ds()
{
  register unsigned16 segment = 0;

  asm volatile ( "movw %%ds,%0" : "=r" (segment) : "0" (segment) );

  return segment;
}

static inline unsigned16 get_es()
{
  register unsigned16 segment = 0;

  asm volatile ( "movw %%es,%0" : "=r" (segment) : "0" (segment) );

  return segment;
}

static inline unsigned16 get_ss()
{
  register unsigned16 segment = 0;

  asm volatile ( "movw %%ss,%0" : "=r" (segment) : "0" (segment) );

  return segment;
}

static inline unsigned16 get_fs()
{
  register unsigned16 segment = 0;

  asm volatile ( "movw %%fs,%0" : "=r" (segment) : "0" (segment) );

  return segment;
}

static inline unsigned16 get_gs()
{
  register unsigned16 segment = 0;

  asm volatile ( "movw %%gs,%0" : "=r" (segment) : "0" (segment) );

  return segment;
}

/* i80x86 I/O instructions */

#define outport_byte( _port, _value ) \
   { register unsigned16 __port  = _port; \
     register unsigned8  __value = _value; \
     \
     asm volatile ( "outb %0,%1" : "=a" (__value), "=d" (__port) \
				 : "0"   (__value), "1"  (__port) \
		  ); \
   }

#define outport_word( _port, _value ) \
   { register unsigned16 __port  = _port; \
     register unsigned16 __value = _value; \
     \
     asm volatile ( "outw %0,%1" : "=a" (__value), "=d" (__port) \
				 : "0"   (__value), "1"  (__port) \
		  ); \
   }

#define outport_long( _port, _value ) \
   { register unsigned16 __port  = _port; \
     register unsigned32 __value = _value; \
     \
     asm volatile ( "outl %0,%1" : "=a" (__value), "=d" (__port) \
				 : "0"   (__value), "1"  (__port) \
		  ); \
   }

#define inport_byte( _port, _value ) \
   { register unsigned16 __port  = _port; \
     register unsigned8  __value = 0; \
     \
     asm volatile ( "inb %1,%0" : "=a" (__value), "=d" (__port) \
				: "0"   (__value), "1"  (__port) \
		  ); \
     _value = __value; \
   }

#define inport_word( _port, _value ) \
   { register unsigned16 __port  = _port; \
     register unsigned16 __value = 0; \
     \
     asm volatile ( "inw %1,%0" : "=a" (__value), "=d" (__port) \
				: "0"   (__value), "1"  (__port) \
		  ); \
     _value = __value; \
   }

#define inport_long( _port, _value ) \
   { register unsigned16 __port  = _port; \
     register unsigned32 __value = 0; \
     \
     asm volatile ( "inl %1,%0" : "=a" (__value), "=d" (__port) \
				: "0"   (__value), "1"  (__port) \
		  ); \
     _value = __value; \
   }

/* structures */

/* See Chapter 5 - Memory Management in i386 manual */

struct GDT_slot {
  unsigned16 limit_0_15;
  unsigned16 base_0_15;
  unsigned8  base_16_23;
  unsigned8  type_dt_dpl_p;
  unsigned8  limit_16_19_granularity;
  unsigned8  base_24_31;
};

/* See Chapter 9 - Exceptions and Interrupts in i386 manual
 *
 *  NOTE: This is the IDT entry for interrupt gates ONLY.
 */

struct IDT_slot {
  unsigned16 offset_0_15;
  unsigned16 segment_selector;
  unsigned8  reserved;
  unsigned8  p_dpl;
  unsigned16 offset_16_31;
};

struct DTR_load_save_format {
  unsigned16 limit;
  unsigned32 physical_address;
};

/* variables */

extern struct IDT_slot Interrupt_descriptor_table[ 256 ];
extern struct GDT_slot Global_descriptor_table[ 8192 ];

/* functions */

#ifdef CPU_INITIALIZE
#define EXTERN
#else
#undef EXTERN
#define EXTERN extern
#endif

void *Logical_to_physical(
  unsigned16  segment,
  void             *address
);

void *Physical_to_logical(
  unsigned16  segment,
  void             *address
);

/* complicated static inline functions */

#define get_GDTR( _gdtr_address ) \
  { \
    void                        *_gdtr = (_gdtr_address); \
    \
    asm volatile( "sgdt   (%0)" : "=r" (_gdtr) : "0" (_gdtr) ); \
  }

#define get_GDT_slot( _gdtr_base, _segment, _slot_address ) \
  { \
    register unsigned32  _gdt_slot  = (_gdtr_base) + (_segment); \
    register volatile void    *_slot      = (_slot_address); \
    register unsigned32  _temporary = 0; \
    \
    asm volatile( "movl %%gs:(%0),%1 ; \
		   movl %1,(%2) ; \
		   movl %%gs:4(%0),%1 ; \
		   movl %1,4(%2)"  \
		     : "=r" (_gdt_slot), "=r" (_temporary), "=r" (_slot) \
		     : "0"  (_gdt_slot), "1"  (_temporary), "2"  (_slot) \
		);  \
  }

#define set_GDT_slot( _gdtr_base, _segment, _slot_address ) \
  { \
    register unsigned32  _gdt_slot  = (_gdtr_base) + (_segment); \
    register volatile void    *_slot      = (_slot_address); \
    register unsigned32  _temporary = 0; \
    \
    asm volatile( "movl (%2),%1 ; \
		   movl %1,%%gs:(%0) ; \
		   movl 4(%2),%1 ; \
		   movl %1,%%gs:4(%0) \
		  " \
		     : "=r" (_gdt_slot), "=r" (_temporary), "=r" (_slot) \
		     : "0"  (_gdt_slot), "1"  (_temporary), "2"  (_slot) \
		);  \
  }

static inline void set_segment(
  unsigned16 segment,
  unsigned32 base,
  unsigned32 limit
)
{
  struct DTR_load_save_format  gdtr;
  volatile struct GDT_slot     Gdt_slot;
  volatile struct GDT_slot    *gdt_slot = &Gdt_slot;
  unsigned16             tmp_segment = 0;
  unsigned32             limit_adjusted;


  /* load physical address of the GDT */

  get_GDTR( &gdtr );

  gdt_slot->type_dt_dpl_p  = 0x92;             /* present, dpl=0,      */
					       /* application=1,       */
					       /* type=data read/write */
  gdt_slot->limit_16_19_granularity = 0x40;    /* 32 bit segment       */

  limit_adjusted = limit;
  if ( limit > 4095 ) {
    gdt_slot->limit_16_19_granularity |= 0x80; /* set granularity bit */
    limit_adjusted /= 4096;
  }

  gdt_slot->limit_16_19_granularity |= (limit_adjusted >> 16) & 0xff;
  gdt_slot->limit_0_15               = limit_adjusted & 0xffff;

  gdt_slot->base_0_15  = base & 0xffff;
  gdt_slot->base_16_23 = (base >> 16) & 0xff;
  gdt_slot->base_24_31 = (base >> 24);

  set_GDT_slot( gdtr.physical_address, segment, gdt_slot );

  /* Now, reload all segment registers so the limit takes effect. */

  asm volatile( "movw %%ds,%0 ; movw %0,%%ds\n"
		"movw %%es,%0 ; movw %0,%%es\n"
		"movw %%fs,%0 ; movw %0,%%fs\n"
		"movw %%gs,%0 ; movw %0,%%gs\n"
		"movw %%ss,%0 ; movw %0,%%ss"
		   : "=r" (tmp_segment)
		   : "0"  (tmp_segment)
	      );

}

#endif
/* end of include file */
