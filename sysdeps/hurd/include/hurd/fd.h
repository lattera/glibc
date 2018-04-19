#ifndef	_HURD_FD_H
#include_next <hurd/fd.h>
#ifndef _ISOMAC
libc_hidden_proto (_hurd_intern_fd)
libc_hidden_proto (_hurd_fd_error)
libc_hidden_proto (_hurd_fd_error_signal)
#  ifdef _HURD_FD_H_HIDDEN_DEF
libc_hidden_def (_hurd_fd_error)
libc_hidden_def (_hurd_fd_error_signal)
#  endif
#endif
#endif
