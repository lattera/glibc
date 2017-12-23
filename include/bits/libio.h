#if !defined _ISOMAC && defined _IO_MTSAFE_IO
# include <stdio-lock.h>
#endif
#include <libio/bits/libio.h>

#ifndef _ISOMAC
#ifndef _LIBC_LIBIO_H
#define _LIBC_LIBIO_H

libc_hidden_proto (__overflow)
libc_hidden_proto (__underflow)
libc_hidden_proto (__uflow)
libc_hidden_proto (__woverflow)
libc_hidden_proto (__wunderflow)
libc_hidden_proto (__wuflow)
libc_hidden_proto (_IO_free_backup_area)
libc_hidden_proto (_IO_free_wbackup_area)
libc_hidden_proto (_IO_padn)
libc_hidden_proto (_IO_putc)
libc_hidden_proto (_IO_sgetn)
libc_hidden_proto (_IO_vfprintf)
libc_hidden_proto (_IO_vfscanf)

#ifdef _IO_MTSAFE_IO
# undef _IO_peekc
# undef _IO_flockfile
# undef _IO_funlockfile
# undef _IO_ftrylockfile

# define _IO_peekc(_fp) _IO_peekc_locked (_fp)
# if _IO_lock_inexpensive
#  define _IO_flockfile(_fp) \
  if (((_fp)->_flags & _IO_USER_LOCK) == 0) _IO_lock_lock (*(_fp)->_lock)
#  define _IO_funlockfile(_fp) \
  if (((_fp)->_flags & _IO_USER_LOCK) == 0) _IO_lock_unlock (*(_fp)->_lock)
# else
#  define _IO_flockfile(_fp) \
  if (((_fp)->_flags & _IO_USER_LOCK) == 0) _IO_flockfile (_fp)
#  define _IO_funlockfile(_fp) \
  if (((_fp)->_flags & _IO_USER_LOCK) == 0) _IO_funlockfile (_fp)
# endif
#endif /* _IO_MTSAFE_IO */

#endif
#endif
