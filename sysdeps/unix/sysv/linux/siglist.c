#include <stddef.h>
#include <signal.h>

const char * const _sys_siglist[NSIG] =
{
#define init_sig(sig, abbrev, desc)   [sig] desc,
#include "siglist.h"
#undef init_sig
};

const char * const _sys_sigabbrev[NSIG] =
{
#define init_sig(sig, abbrev, desc)   [sig] abbrev,
#include "siglist.h"
#undef init_sig
};

weak_alias(_sys_siglist, sys_siglist)
weak_alias(_sys_sigabbrev, sys_sigabbrev)
