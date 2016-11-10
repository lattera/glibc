/* Using NaCl interface tables.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _NACL_INTERFACES_H
#define _NACL_INTERFACES_H	1

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* <nacl-irt.h> is massaged from native_client/src/untrusted/irt/irt.h so
   that it uses nacl_abi_*_t type names.  We must define those types first.  */

/* These are the same in the IRT ABI as in the libc ABI.  */
typedef blksize_t nacl_abi_blksize_t;
typedef dev_t nacl_abi_dev_t;
typedef gid_t nacl_abi_gid_t;
typedef ino_t nacl_abi_ino_t;
typedef mode_t nacl_abi_mode_t;
typedef nlink_t nacl_abi_nlink_t;
typedef size_t nacl_abi_size_t;
typedef time_t nacl_abi_time_t;
typedef uid_t nacl_abi_uid_t;
typedef struct dirent nacl_abi_dirent_t;
typedef struct timeval nacl_abi_timeval_t;
typedef struct timespec nacl_abi_timespec_t;

/* This is unsigned in the IRT ABI, but it's traditionally 'long int',
   so we stick with that.  */
typedef clock_t nacl_abi_clock_t;

typedef int32_t nacl_abi_blkcnt_t;

/* This is different by design.  */
typedef struct nacl_abi_stat nacl_abi_stat_t;

#include <nacl-irt.h>

/* This is how we access the IRT interface-query function.
   This formulation makes it usable as if it were a function name.  */
#define __nacl_irt_query	(*(TYPE_nacl_irt_query) GLRO(dl_sysinfo))


/* This describes one IRT (or IRT-like) interface that libc uses.
   This structure contains no pointers, so it can go into rodata
   without relocs.  Instead, the actual tables we use for these
   interfaces live in a parallel section in writable data.  */
struct nacl_interface {
  size_t table_size;
  size_t namelen;
  char name[];
};

/* Increment for 'const struct nacl_interface *' pointers.  */
static inline const struct nacl_interface *
next_nacl_interface (const struct nacl_interface *i)
{
  uintptr_t align = __alignof (*i);
  return (const void *) (((uintptr_t) &i->name[i->namelen] + align - 1)
			 & -align);
}

#if IS_IN (libpthread)
# define libpthread_hidden_proto(name)	hidden_proto (name)
#else
# define libpthread_hidden_proto(name)
#endif

#define DECLARE_INTERFACE(module, type) \
  extern struct type __##type; module##_hidden_proto (__##type);

#define NACL_MANDATORY_INTERFACE(module, id, type)	\
  DECLARE_INTERFACE (module, type)
#define NACL_OPTIONAL_INTERFACE(module, id, type)	\
  DECLARE_INTERFACE (module, type)
#include "nacl-interface-list.h"
#undef	NACL_MANDATORY_INTERFACE
#undef	NACL_OPTIONAL_INTERFACE

extern void __nacl_initialize_interfaces (void) attribute_hidden;
extern bool __nacl_supply_interface_libc (const char *ident, size_t ident_len,
					  const void *table, size_t tablesize)
  internal_function attribute_hidden;
extern bool __nacl_supply_interface_rtld (const char *ident, size_t ident_len,
					  const void *table, size_t tablesize);
  internal_function;

/* Convenience function for handling IRT call return values.  */
static inline int
__nacl_fail (int err)
{
  errno = err;
  return -1;
}

#define NACL_CALL(err, val) \
  ({ int _err = (err); _err ? __nacl_fail (_err) : (val); })

/* Same as NACL_CALL but without setting errno.  */
#define NACL_CALL_NOERRNO(err, val) \
  ({ int _err = (err); _err ? _err : (val); })

#endif  /* nacl-interfaces.h */
