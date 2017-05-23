/* Architecture-specific adjustments to siginfo_t.  SPARC version.  */
#ifndef _BITS_SIGINFO_ARCH_H
#define _BITS_SIGINFO_ARCH_H 1

#define __SI_BAND_TYPE int

#define __SI_SIGFAULT_ADDL \
  int _si_trapno;

#define si_trapno	_sifields._sigfault._si_trapno

#endif
