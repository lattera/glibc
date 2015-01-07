/* Private floating point rounding and exceptions handling. PowerPC version.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef FENV_PRIVATE_H
#define FENV_PRIVATE_H 1

#include <fenv.h>
#include <fenv_libc.h>
#include <fpu_control.h>

#define _FPU_MASK_ALL (_FPU_MASK_ZM | _FPU_MASK_OM | _FPU_MASK_UM \
                      | _FPU_MASK_XM | _FPU_MASK_IM)

/* Mask everything but the rounding moded and non-IEEE arithmetic flags.  */
#define _FPU_MASK_ROUNDING 0xffffffff00000007LL

/* Mask restore rounding mode and exception enabled.  */
#define _FPU_MASK_EXCEPT_ROUND 0xffffffff1fffff00LL

/* Mask exception enable but fraction rounded/inexact and FP result/CC
   bits.  */
#define _FPU_MASK_FRAC_INEX_RET_CC 0x1ff80fff

static __always_inline void
libc_feholdexcept_ppc (fenv_t *envp)
{
  fenv_union_t old, new;

  old.fenv = *envp = fegetenv_register ();

  new.l = old.l & _FPU_MASK_ROUNDING;

  /* If the old env had any enabled exceptions, then mask SIGFPE in the
     MSR FE0/FE1 bits.  This may allow the FPU to run faster because it
     always takes the default action and can not generate SIGFPE.  */
  if ((old.l & _FPU_MASK_ALL) != 0)
    (void) __fe_mask_env ();

  fesetenv_register (new.fenv);
}

static __always_inline void
libc_fesetround_ppc (int r)
{
  __fesetround_inline (r);
}

static __always_inline void
libc_feholdexcept_setround_ppc (fenv_t *envp, int r)
{
  fenv_union_t old, new;

  old.fenv = *envp = fegetenv_register ();

  new.l = (old.l & _FPU_MASK_ROUNDING) | r;

  if ((old.l & _FPU_MASK_ALL) != 0)
    (void) __fe_mask_env ();

  fesetenv_register (new.fenv);
}

static __always_inline int
libc_fetestexcept_ppc (int e)
{
  fenv_union_t u;
  u.fenv = fegetenv_register ();
  return u.l & e;
}

static __always_inline void
libc_fesetenv_ppc (const fenv_t *envp)
{
  fenv_union_t old, new;

  new.fenv = *envp;
  old.fenv = fegetenv_register ();

  /* If the old env has no enabled exceptions and the new env has any enabled
     exceptions, then unmask SIGFPE in the MSR FE0/FE1 bits.  This will put the
     hardware into "precise mode" and may cause the FPU to run slower on some
     hardware.  */
  if ((old.l & _FPU_MASK_ALL) == 0 && (new.l & _FPU_MASK_ALL) != 0)
    (void) __fe_nomask_env_priv ();

  /* If the old env had any enabled exceptions and the new env has no enabled
     exceptions, then mask SIGFPE in the MSR FE0/FE1 bits.  This may allow the
     FPU to run faster because it always takes the default action and can not
     generate SIGFPE.  */
  if ((old.l & _FPU_MASK_ALL) != 0 && (new.l & _FPU_MASK_ALL) == 0)
    (void) __fe_mask_env ();

  fesetenv_register (*envp);
}

static __always_inline int
libc_feupdateenv_test_ppc (fenv_t *envp, int ex)
{
  fenv_union_t old, new;

  new.fenv = *envp;
  old.fenv = fegetenv_register ();

  /* Restore rounding mode and exception enable from *envp and merge
     exceptions.  Leave fraction rounded/inexact and FP result/CC bits
     unchanged.  */
  new.l = (old.l & _FPU_MASK_EXCEPT_ROUND)
          | (new.l & _FPU_MASK_FRAC_INEX_RET_CC);

  if ((old.l & _FPU_MASK_ALL) == 0 && (new.l & _FPU_MASK_ALL) != 0)
    (void) __fe_nomask_env_priv ();

  if ((old.l & _FPU_MASK_ALL) != 0 && (new.l & _FPU_MASK_ALL) == 0)
    (void) __fe_mask_env ();

  fesetenv_register (new.fenv);

  return old.l & ex;
}

static __always_inline void
libc_feupdateenv_ppc (fenv_t *e)
{
  libc_feupdateenv_test_ppc (e, 0);
}

static __always_inline void
libc_feholdsetround_ppc (fenv_t *e, int r)
{
  fenv_union_t old, new;

  old.fenv = fegetenv_register ();
  /* Clear current precision and set newer one.  */
  new.l = (old.l & ~0x3) | r;
  *e = old.fenv;

  if ((old.l & _FPU_MASK_ALL) != 0)
    (void) __fe_mask_env ();
  fesetenv_register (new.fenv);
}

static __always_inline void
libc_feresetround_ppc (fenv_t *envp)
{
  fenv_union_t old, new;

  new.fenv = *envp;
  old.fenv = fegetenv_register ();

  /* Restore rounding mode and exception enable from *envp and merge
     exceptions.  Leave fraction rounded/inexact and FP result/CC bits
     unchanged.  */
  new.l = (old.l & _FPU_MASK_EXCEPT_ROUND)
          | (new.l & _FPU_MASK_FRAC_INEX_RET_CC);

  if ((old.l & _FPU_MASK_ALL) == 0 && (new.l & _FPU_MASK_ALL) != 0)
    (void) __fe_nomask_env_priv ();

  if ((old.l & _FPU_MASK_ALL) != 0 && (new.l & _FPU_MASK_ALL) == 0)
    (void) __fe_mask_env ();

  /* Atomically enable and raise (if appropriate) exceptions set in `new'.  */
  fesetenv_register (new.fenv);
}

#define libc_feholdexceptf           libc_feholdexcept_ppc
#define libc_feholdexcept            libc_feholdexcept_ppc
#define libc_feholdexcept_setroundf  libc_feholdexcept_setround_ppc
#define libc_feholdexcept_setround   libc_feholdexcept_setround_ppc
#define libc_fetestexceptf           libc_fetestexcept_ppc
#define libc_fetestexcept            libc_fetestexcept_ppc
#define libc_fesetroundf             libc_fesetround_ppc
#define libc_fesetround              libc_fesetround_ppc
#define libc_fesetenvf               libc_fesetenv_ppc
#define libc_fesetenv                libc_fesetenv_ppc
#define libc_feupdateenv_testf       libc_feupdateenv_test_ppc
#define libc_feupdateenv_test        libc_feupdateenv_test_ppc
#define libc_feupdateenvf            libc_feupdateenv_ppc
#define libc_feupdateenv             libc_feupdateenv_ppc
#define libc_feholdsetroundf         libc_feholdsetround_ppc
#define libc_feholdsetround          libc_feholdsetround_ppc
#define libc_feresetroundf           libc_feresetround_ppc
#define libc_feresetround            libc_feresetround_ppc


/* We have support for rounding mode context.  */
#define HAVE_RM_CTX 1

static __always_inline void
libc_feholdexcept_setround_ppc_ctx (struct rm_ctx *ctx, int r)
{
  fenv_union_t old, new;

  old.fenv = fegetenv_register ();

  new.l = (old.l & _FPU_MASK_ROUNDING) | r;
  ctx->env = old.fenv;
  if (__glibc_unlikely (new.l != old.l))
    {
      if ((old.l & _FPU_MASK_ALL) != 0)
	(void) __fe_mask_env ();
      fesetenv_register (new.fenv);
      ctx->updated_status = true;
    }
  else
    ctx->updated_status = false;
}

static __always_inline void
libc_fesetenv_ppc_ctx (struct rm_ctx *ctx)
{
  libc_fesetenv_ppc (&ctx->env);
}

static __always_inline void
libc_feupdateenv_ppc_ctx (struct rm_ctx *ctx)
{
  if (__glibc_unlikely (ctx->updated_status))
    libc_feupdateenv_test_ppc (&ctx->env, 0);
}

static __always_inline void
libc_feholdsetround_ppc_ctx (struct rm_ctx *ctx, int r)
{
  fenv_union_t old, new;

  old.fenv = fegetenv_register ();
  new.l = (old.l & ~0x3) | r;
  ctx->env = old.fenv;
  if (__glibc_unlikely (new.l != old.l))
    {
      if ((old.l & _FPU_MASK_ALL) != 0)
	(void) __fe_mask_env ();
      fesetenv_register (new.fenv);
      ctx->updated_status = true;
    }
  else
    ctx->updated_status = false;
}

static __always_inline void
libc_feresetround_ppc_ctx (struct rm_ctx *ctx)
{
  if (__glibc_unlikely (ctx->updated_status))
    libc_feresetround_ppc (&ctx->env);
}

#define libc_feholdexcept_setround_ctx   libc_feholdexcept_setround_ppc_ctx
#define libc_feholdexcept_setroundf_ctx  libc_feholdexcept_setround_ppc_ctx
#define libc_feholdexcept_setroundl_ctx  libc_feholdexcept_setround_ppc_ctx
#define libc_fesetenv_ctx                libc_fesetenv_ppc_ctx
#define libc_fesetenvf_ctx               libc_fesetenv_ppc_ctx
#define libc_fesetenvl_ctx               libc_fesetenv_ppc_ctx
#define libc_feholdsetround_ctx          libc_feholdsetround_ppc_ctx
#define libc_feholdsetroundf_ctx         libc_feholdsetround_ppc_ctx
#define libc_feholdsetroundl_ctx         libc_feholdsetround_ppc_ctx
#define libc_feresetround_ctx            libc_feresetround_ppc_ctx
#define libc_feresetroundf_ctx           libc_feresetround_ppc_ctx
#define libc_feresetroundl_ctx           libc_feresetround_ppc_ctx
#define libc_feupdateenv_ctx             libc_feupdateenv_ppc_ctx
#define libc_feupdateenvf_ctx            libc_feupdateenv_ppc_ctx
#define libc_feupdateenvl_ctx            libc_feupdateenv_ppc_ctx

#endif
