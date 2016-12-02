/* Architecture-specific additional siginfo constants.  ia64 version.  */
#ifndef _BITS_SIGINFO_CONSTS_ARCH_H
#define _BITS_SIGINFO_CONSTS_ARCH_H 1

/* `si_code' values for SIGILL signal.  */
enum
{
  ILL_BADIADDR = ILL_BADSTK + 1, /* Unimplemented instruction address. */
#define ILL_BADIADDR ILL_BADIADDR
  ILL_BREAK
#define ILL_BREAK ILL_BREAK
};

/* `si_code' values for SIGFPE signal.  */
enum
{
   FPE_DECOVF   = FPE_FLTSUB + 1,
#define FPE_DECOVF  FPE_DECOVF
   FPE_DECDIV,
#define FPE_DECDIV  FPE_DECDIV
   FPE_DECERR,
#define FPE_DECERR  FPE_DECERR
   FPE_INVASC,
#define FPE_INVASC  FPE_INVASC
   FPE_INVDEC
#define FPE_INVDEC  FPE_INVDEC
};

/* `si_code' values for SIGSEGV signal.  */
enum
{
  SEGV_PSTKOVF = SEGV_ACCERR + 1
#define SEGV_PSTKOVF SEGV_PSTKOVF
};

/* `si_code' values for SIGTRAP signal.  */
enum
{
  TRAP_BRANCH = TRAP_TRACE + 1,
#define TRAP_BRANCH TRAP_BRANCH
  TRAP_HWBKPT
#define TRAP_HWBKPT TRAP_HWBKPT
};

#endif
