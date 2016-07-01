/* PLT trampolines.  x86-64 version.
   Copyright (C) 2004-2016 Free Software Foundation, Inc.
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

#include <config.h>
#include <sysdep.h>
#include <link-defines.h>

#ifndef DL_STACK_ALIGNMENT
/* Due to GCC bug:

   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58066

   __tls_get_addr may be called with 8-byte stack alignment.  Although
   this bug has been fixed in GCC 4.9.4, 5.3 and 6, we can't assume
   that stack will be always aligned at 16 bytes.  We use unaligned
   16-byte move to load and store SSE registers, which has no penalty
   on modern processors if stack is 16-byte aligned.  */
# define DL_STACK_ALIGNMENT 8
#endif

#ifndef DL_RUNTIME_UNALIGNED_VEC_SIZE
/* The maximum size in bytes of unaligned vector load and store in the
   dynamic linker.  Since SSE optimized memory/string functions with
   aligned SSE register load and store are used in the dynamic linker,
   we must set this to 8 so that _dl_runtime_resolve_sse will align the
   stack before calling _dl_fixup.  */
# define DL_RUNTIME_UNALIGNED_VEC_SIZE 8
#endif

/* True if _dl_runtime_resolve should align stack to VEC_SIZE bytes.  */
#define DL_RUNTIME_RESOLVE_REALIGN_STACK \
  (VEC_SIZE > DL_STACK_ALIGNMENT \
   && VEC_SIZE > DL_RUNTIME_UNALIGNED_VEC_SIZE)

/* Align vector register save area to 16 bytes.  */
#define REGISTER_SAVE_VEC_OFF	0

/* Area on stack to save and restore registers used for parameter
   passing when calling _dl_fixup.  */
#ifdef __ILP32__
# define REGISTER_SAVE_RAX	(REGISTER_SAVE_VEC_OFF + VEC_SIZE * 8)
# define PRESERVE_BND_REGS_PREFIX
#else
/* Align bound register save area to 16 bytes.  */
# define REGISTER_SAVE_BND0	(REGISTER_SAVE_VEC_OFF + VEC_SIZE * 8)
# define REGISTER_SAVE_BND1	(REGISTER_SAVE_BND0 + 16)
# define REGISTER_SAVE_BND2	(REGISTER_SAVE_BND1 + 16)
# define REGISTER_SAVE_BND3	(REGISTER_SAVE_BND2 + 16)
# define REGISTER_SAVE_RAX	(REGISTER_SAVE_BND3 + 16)
# ifdef HAVE_MPX_SUPPORT
#  define PRESERVE_BND_REGS_PREFIX bnd
# else
#  define PRESERVE_BND_REGS_PREFIX .byte 0xf2
# endif
#endif
#define REGISTER_SAVE_RCX	(REGISTER_SAVE_RAX + 8)
#define REGISTER_SAVE_RDX	(REGISTER_SAVE_RCX + 8)
#define REGISTER_SAVE_RSI	(REGISTER_SAVE_RDX + 8)
#define REGISTER_SAVE_RDI	(REGISTER_SAVE_RSI + 8)
#define REGISTER_SAVE_R8	(REGISTER_SAVE_RDI + 8)
#define REGISTER_SAVE_R9	(REGISTER_SAVE_R8 + 8)

#define RESTORE_AVX

#define VEC_SIZE		64
#define VMOVA			vmovdqa64
#if DL_RUNTIME_RESOLVE_REALIGN_STACK || VEC_SIZE <= DL_STACK_ALIGNMENT
# define VMOV			vmovdqa64
#else
# define VMOV			vmovdqu64
#endif
#define VEC(i)			zmm##i
#define _dl_runtime_resolve	_dl_runtime_resolve_avx512
#define _dl_runtime_profile	_dl_runtime_profile_avx512
#include "dl-trampoline.h"
#undef _dl_runtime_resolve
#undef _dl_runtime_profile
#undef VEC
#undef VMOV
#undef VMOVA
#undef VEC_SIZE

#define VEC_SIZE		32
#define VMOVA			vmovdqa
#if DL_RUNTIME_RESOLVE_REALIGN_STACK || VEC_SIZE <= DL_STACK_ALIGNMENT
# define VMOV			vmovdqa
#else
# define VMOV			vmovdqu
#endif
#define VEC(i)			ymm##i
#define _dl_runtime_resolve	_dl_runtime_resolve_avx
#define _dl_runtime_profile	_dl_runtime_profile_avx
#include "dl-trampoline.h"
#undef _dl_runtime_resolve
#undef _dl_runtime_profile
#undef VEC
#undef VMOV
#undef VMOVA
#undef VEC_SIZE

/* movaps/movups is 1-byte shorter.  */
#define VEC_SIZE		16
#define VMOVA			movaps
#if DL_RUNTIME_RESOLVE_REALIGN_STACK || VEC_SIZE <= DL_STACK_ALIGNMENT
# define VMOV			movaps
#else
# define VMOV			movups
#endif
#define VEC(i)			xmm##i
#define _dl_runtime_resolve	_dl_runtime_resolve_sse
#define _dl_runtime_profile	_dl_runtime_profile_sse
#undef RESTORE_AVX
#include "dl-trampoline.h"
