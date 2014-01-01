/* Internal math stuff.  MIPS version.
   Copyright (C) 2013-2014 Free Software Foundation, Inc.
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

#ifndef _MATH_PRIVATE_H

#ifdef __mips_nan2008
/* MIPS aligned to IEEE 754-2008.  */
#else
/* One of the few architectures where the meaning of the quiet/signaling bit is
   inverse to IEEE 754-2008 (as well as common practice for IEEE 754-1985).  */
# define HIGH_ORDER_BIT_IS_SET_FOR_SNAN
#endif

/* Inline functions to speed up the math library implementation.  The
   default versions of these routines are in generic/math_private.h
   and call fesetround, feholdexcept, etc.  These routines use inlined
   code instead.  */

#ifdef __mips_hard_float

# include <fenv.h>
# include <fenv_libc.h>
# include <fpu_control.h>

static __always_inline void
libc_feholdexcept_mips (fenv_t *envp)
{
  fpu_control_t cw;

  /* Save the current state.  */
  _FPU_GETCW (cw);
  envp->__fp_control_register = cw;

  /* Clear all exception enable bits and flags.  */
  cw &= ~(_FPU_MASK_V|_FPU_MASK_Z|_FPU_MASK_O|_FPU_MASK_U|_FPU_MASK_I|FE_ALL_EXCEPT);
  _FPU_SETCW (cw);
}
# define libc_feholdexcept libc_feholdexcept_mips
# define libc_feholdexceptf libc_feholdexcept_mips
# define libc_feholdexceptl libc_feholdexcept_mips

static __always_inline void
libc_fesetround_mips (int round)
{
  fpu_control_t cw;

  /* Get current state.  */
  _FPU_GETCW (cw);

  /* Set rounding bits.  */
  cw &= ~_FPU_RC_MASK;
  cw |= round;

  /* Set new state.  */
  _FPU_SETCW (cw);
}
# define libc_fesetround libc_fesetround_mips
# define libc_fesetroundf libc_fesetround_mips
# define libc_fesetroundl libc_fesetround_mips

static __always_inline void
libc_feholdexcept_setround_mips (fenv_t *envp, int round)
{
  fpu_control_t cw;

  /* Save the current state.  */
  _FPU_GETCW (cw);
  envp->__fp_control_register = cw;

  /* Clear all exception enable bits and flags.  */
  cw &= ~(_FPU_MASK_V|_FPU_MASK_Z|_FPU_MASK_O|_FPU_MASK_U|_FPU_MASK_I|FE_ALL_EXCEPT);

  /* Set rounding bits.  */
  cw &= ~_FPU_RC_MASK;
  cw |= round;

  /* Set new state.  */
  _FPU_SETCW (cw);
}
# define libc_feholdexcept_setround libc_feholdexcept_setround_mips
# define libc_feholdexcept_setroundf libc_feholdexcept_setround_mips
# define libc_feholdexcept_setroundl libc_feholdexcept_setround_mips

static __always_inline void
libc_fesetenv_mips (fenv_t *envp)
{
  fpu_control_t cw;

  /* Read current state to flush fpu pipeline.  */
  _FPU_GETCW (cw);

  _FPU_SETCW (envp->__fp_control_register);
}
# define libc_fesetenv libc_fesetenv_mips
# define libc_fesetenvf libc_fesetenv_mips
# define libc_fesetenvl libc_fesetenv_mips

static __always_inline void
libc_feupdateenv_mips (fenv_t *envp)
{
  int temp;

  /* Save current exceptions.  */
  _FPU_GETCW (temp);

  /* Set flag bits (which are accumulative), and *also* set the
     cause bits.  The setting of the cause bits is what actually causes
     the hardware to generate the exception, if the corresponding enable
     bit is set as well.  */
  temp &= FE_ALL_EXCEPT;
  temp |= envp->__fp_control_register | (temp << CAUSE_SHIFT);

  /* Set new state.  */
  _FPU_SETCW (temp);
}
# define libc_feupdateenv libc_feupdateenv_mips
# define libc_feupdateenvf libc_feupdateenv_mips
# define libc_feupdateenvl libc_feupdateenv_mips

#endif

#include_next <math_private.h>

#endif
