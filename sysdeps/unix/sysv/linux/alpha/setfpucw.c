#include <fpu_control.h>

#include <asm/fpu.h>

extern void		__ieee_set_fp_control (unsigned long);
extern unsigned long	__ieee_get_fp_control (void);


static inline unsigned long
rdfpcr (void)
{
    unsigned long fpcr;

    asm ("trapb; mf_fpcr $f0; trapb; stt $f0,%0" : "m="(fpcr));
    return fpcr;
}


static inline void
wrfpcr (unsigned long fpcr)
{
    asm volatile ("ldt $f0,%0; trapb; mt_fpcr $f0; trapb" :: "m"(fpcr));
}


void
__setfpucw (unsigned short fpu_control)
{
    unsigned long fpcr = 0, fpcw = 0;

    if (!fpu_control)
	fpu_control = _FPU_DEFAULT;

    /* first, set dynamic rounding mode: */

    fpcr = rdfpcr();
    fpcr &= ~FPCR_DYN_MASK;
    switch (fpu_control & 0xc00) {
      case _FPU_RC_NEAREST:	fpcr |= FPCR_DYN_NORMAL; break;
      case _FPU_RC_DOWN:	fpcr |= FPCR_DYN_MINUS; break;
      case _FPU_RC_UP:		fpcr |= FPCR_DYN_PLUS; break;
      case _FPU_RC_ZERO:	fpcr |= FPCR_DYN_CHOPPED; break;
    }
    wrfpcr(fpcr);

    /* now tell kernel about traps that we like to hear about: */

    fpcw = __ieee_get_fp_control();
    fpcw &= ~IEEE_TRAP_ENABLE_MASK;

    if (!(fpu_control & _FPU_MASK_IM))
	fpcw |= IEEE_TRAP_ENABLE_INV;
    if (!(fpu_control & _FPU_MASK_DM))
	fpcw |= IEEE_TRAP_ENABLE_UNF;
    if (!(fpu_control & _FPU_MASK_ZM))
	fpcw |= IEEE_TRAP_ENABLE_DZE;
    if (!(fpu_control & _FPU_MASK_OM))
	fpcw |= IEEE_TRAP_ENABLE_OVF;
    if (!(fpu_control & _FPU_MASK_PM))
	fpcw |= IEEE_TRAP_ENABLE_INE;

    __ieee_set_fp_control(fpcw);

    __fpu_control = fpu_control;	/* update global copy */
}
