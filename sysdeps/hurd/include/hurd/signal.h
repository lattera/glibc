#ifndef	_HURD_SIGNAL_H
#include_next <hurd/signal.h>
#ifndef _ISOMAC
libc_hidden_proto (_hurd_exception2signal)
libc_hidden_proto (_hurd_intr_rpc_mach_msg)
libc_hidden_proto (_hurd_thread_sigstate)
libc_hidden_proto (_hurd_raise_signal)
#endif
#endif
