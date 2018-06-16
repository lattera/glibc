#ifndef	_HURD_SIGNAL_H
extern struct hurd_sigstate *_hurd_self_sigstate (void) __attribute__ ((__const__));
#ifndef _ISOMAC
libc_hidden_proto (_hurd_self_sigstate)
#endif

#include_next <hurd/signal.h>

#ifndef _ISOMAC
libc_hidden_proto (_hurd_exception2signal)
libc_hidden_proto (_hurd_intr_rpc_mach_msg)
libc_hidden_proto (_hurd_thread_sigstate)
libc_hidden_proto (_hurd_raise_signal)
#endif
#ifdef _HURD_SIGNAL_H_HIDDEN_DEF
libc_hidden_def (_hurd_self_sigstate)
#endif
#endif
