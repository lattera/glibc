/* This file is needed by libio to define various configuration parameters.
   These are always the same in the GNU C library.  */

#ifndef _G_config_h
#define _G_config_h 1

/* Define types for libio in terms of the standard internal type names.  */

#include <gnu/types.h>
#define __need_size_t
#include <stddef.h>
#define _G_size_t	size_t
#define _G_fpos_t	__off_t
#define _G_ssize_t	__ssize_t
#define _G_off_t	__off_t
#define	_G_pid_t	__pid_t
#define	_G_uid_t	__uid_t


/* These library features are always available in the GNU C library.  */
#define _G_HAVE_ATEXIT 1
#define _G_HAVE_SYS_WAIT 1
#define _G_NEED_STDARG_H 1
#define _G_va_list __gnuc_va_list

/* This is defined by <statbuf.h> if `st_blksize' exists.  */
#define _G_HAVE_ST_BLKSIZE defined (_STATBUF_ST_BLKSIZE)

#define _G_BUFSIZ 8192

/* These are the vtbl details for ELF.  */
#define _G_NAMES_HAVE_UNDERSCORE 0
#define _G_VTABLE_LABEL_PREFIX "_vt."
#define _G_VTABLE_LABEL_HAS_LENGTH 1


#endif	/* _G_config.h */
