/* Floating-point constants for the 68881.
   Copyright (C) 1992 Free Software Foundation, Inc.  */

/* IGNORE($ This is used internally in the library.  */
#include <sysdeps/ieee754/fl.h>
/* ansidecl.m4 here inserts the ieee file.  Kludge o rama.
   $) ENDCOMMENT INCLUDE($sysdeps/ieee754/fl.h$) STARTCOMMENT */

#ifndef	__need_HUGE_VAL

#ifdef	__GNUC__

#undef	FLT_ROUNDS

/* Interrogate the 68881 to find the current rounding mode.  */

static __const __inline int
DEFUN_VOID(__flt_rounds)
{
  unsigned long int __fpcr;
  __asm("fmove%.l fpcr, %0" : "=g" (__fpcr));
  switch (__fpcr & (1 | 2))
    {
    case 0:
      return _FLT_ROUNDS_TONEAREST;
    case 1:
      return _FLT_ROUNDS_TOZERO;
    case 2:
      return _FLT_ROUNDS_TONEGINF;
    case 3:
      return _FLT_ROUNDS_TOPOSINF;
    default:
      return _FLT_ROUNDS_INDETERMINATE;
    }
}

#define	FLT_ROUNDS	(__flt_rounds())

#endif	/* GCC.  */

#endif	/* Don't need HUGE_VAL.  */
