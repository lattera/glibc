/* Shared RTM header.  Emulate TSX intrinsics for compilers and assemblers
   that do not support the intrinsics and instructions yet.  */
#ifndef _HLE_H
#define _HLE_H 1

#ifdef __ASSEMBLER__

.macro XBEGIN target
	.byte 0xc7,0xf8
	.long \target-1f
1:
.endm

.macro XEND
	.byte 0x0f,0x01,0xd5
.endm

.macro XABORT code
	.byte 0xc6,0xf8,\code
.endm

.macro XTEST
	 .byte 0x0f,0x01,0xd6
.endm

#endif

/* Official RTM intrinsics interface matching gcc/icc, but works
   on older gcc compatible compilers and binutils.
   We should somehow detect if the compiler supports it, because
   it may be able to generate slightly better code.  */

#define _XBEGIN_STARTED		(~0u)
#define _XABORT_EXPLICIT	(1 << 0)
#define _XABORT_RETRY		(1 << 1)
#define _XABORT_CONFLICT	(1 << 2)
#define _XABORT_CAPACITY	(1 << 3)
#define _XABORT_DEBUG		(1 << 4)
#define _XABORT_NESTED		(1 << 5)
#define _XABORT_CODE(x)		(((x) >> 24) & 0xff)

#define _ABORT_LOCK_BUSY 	0xff
#define _ABORT_LOCK_IS_LOCKED	0xfe
#define _ABORT_NESTED_TRYLOCK	0xfd

#ifndef __ASSEMBLER__

#define __force_inline __attribute__((__always_inline__)) inline

static __force_inline int _xbegin(void)
{
  int ret = _XBEGIN_STARTED;
  asm volatile (".byte 0xc7,0xf8 ; .long 0" : "+a" (ret) :: "memory");
  return ret;
}

static __force_inline void _xend(void)
{
  asm volatile (".byte 0x0f,0x01,0xd5" ::: "memory");
}

static __force_inline void _xabort(const unsigned int status)
{
  asm volatile (".byte 0xc6,0xf8,%P0" :: "i" (status) : "memory");
}

static __force_inline int _xtest(void)
{
  unsigned char out;
  asm volatile (".byte 0x0f,0x01,0xd6 ; setnz %0" : "=r" (out) :: "memory");
  return out;
}

#endif
#endif
