/* Architecture-specific additional siginfo constants.  Tile version.  */
#ifndef _BITS_SIGINFO_CONSTS_ARCH_H
#define _BITS_SIGINFO_CONSTS_ARCH_H 1

/* `si_code' values for SIGILL signal.  */
enum
{
  ILL_DBLFLT = ILL_BADSTK + 1,	/* Double fault.  */
#define ILL_DBLFLT ILL_DBLFLT
  ILL_HARDWALL			/* User networks hardwall violation.  */
#define ILL_HARDWALL ILL_HARDWALL
};

#endif
