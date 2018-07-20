#ifndef	_HURD_PORT_H
#include_next <hurd/port.h>
#ifndef _ISOMAC
libc_hidden_proto (_hurd_port_locked_get)
libc_hidden_proto (_hurd_port_locked_set)
#ifdef _HURD_PORT_H_HIDDEN_DEF
libc_hidden_def (_hurd_port_locked_get)
libc_hidden_def (_hurd_port_locked_set)
#endif
#endif
#endif
