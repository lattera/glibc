#include <stddef.h>
#include <signal.h>

const char * const __new_sys_siglist[NSIG] =
{
#define init_sig(sig, abbrev, desc)   [sig] desc,
#include "siglist.h"
#undef init_sig
};

const char * const __new_sys_sigabbrev[NSIG] =
{
#define init_sig(sig, abbrev, desc)   [sig] abbrev,
#include "siglist.h"
#undef init_sig
};

#ifdef DO_VERSIONING
strong_alias (__new_sys_siglist, _new_sys_siglist)
default_symbol_version (__new_sys_siglist, _sys_siglist, GLIBC_2.1);
default_symbol_version (_new_sys_siglist, sys_siglist, GLIBC_2.1);
default_symbol_version (__new_sys_sigabbrev, sys_sigabbrev, GLIBC_2.1);
#else
weak_alias(_sys_siglist, sys_siglist)
weak_alias(_sys_sigabbrev, sys_sigabbrev)
#endif
