#include <stddef.h>
#include <signal.h>

const char * const __old_sys_siglist[32] =
{
#define init_sig(sig, abbrev, desc)   [sig] desc,
#include "siglist.h"
#undef init_sig
};

const char * const __old_sys_sigabbrev[32] =
{
#define init_sig(sig, abbrev, desc)   [sig] abbrev,
#include "siglist.h"
#undef init_sig
};

strong_alias (__old_sys_siglist, _old_sys_siglist)
symbol_version (__old_sys_siglist, _sys_siglist, GLIBC_2.0);
symbol_version (_old_sys_siglist, sys_siglist, GLIBC_2.0);
symbol_version (__old_sys_sigabbrev, sys_sigabbrev, GLIBC_2.0);
