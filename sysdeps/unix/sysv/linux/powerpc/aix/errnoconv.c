/* Convert the error number the AIX kernel returns to what the Linux
   application expects.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include "linux-errno.h"


static int mapping[] =
{
  [AIX_EPERM] = EPERM,
  [AIX_ENOENT] = ENOENT,
  [AIX_ESRCH] = ESRCH,
  [AIX_EINTR] = EINTR,
  [AIX_EIO] = EIO,
  [AIX_ENXIO] = ENXIO,
  [AIX_E2BIG] = E2BIG,
  [AIX_ENOEXEC] = ENOEXEC,
  [AIX_EBADF] = EBADF,
  [AIX_ECHILD] = ECHILD,
  [AIX_EAGAIN] = EAGAIN,
  [AIX_ENOMEM] = ENOMEM,
  [AIX_EACCES] = EACCES,
  [AIX_EFAULT] = EFAULT,
  [AIX_ENOTBLK] = ENOTBLK,
  [AIX_EBUSY] = EBUSY,
  [AIX_EEXIST] = EEXIST,
  [AIX_EXDEV] = EXDEV,
  [AIX_ENODEV] = ENODEV,
  [AIX_ENOTDIR] = ENOTDIR,
  [AIX_EISDIR] = EISDIR,
  [AIX_EINVAL] = EINVAL,
  [AIX_ENFILE] = ENFILE,
  [AIX_EMFILE] = EMFILE,
  [AIX_ENOTTY] = ENOTTY,
  [AIX_ETXTBSY] = ETXTBSY,
  [AIX_EFBIG] = EFBIG,
  [AIX_ENOSPC] = ENOSPC,
  [AIX_EIDRM] = EIDRM,
  [AIX_ECHRNG] = ECHRNG,
  [AIX_EL2NSYNC] = EL2NSYNC,
  [AIX_EL3HLT] = EL3HLT,
  [AIX_EL3RST] = EL3RST,
  [AIX_ELNRNG] = ELNRNG,
  [AIX_EUNATCH] = EUNATCH,
  [AIX_ENOCSI] = ENOCSI,
  [AIX_EL2HLT] = EL2HLT,
  [AIX_EDEADLK] = EDEADLK,
  [AIX_ENOTREADY] = ENOTREADY,
  // EWPROTECT: no Linux equivalent
  // EFORMAT: no Linux equivalent
  [AIX_ENOLCK] = ENOLCK,
  // ENOCONNECT: No Linux equivalent
  [AIX_ESTALE] = ESTALE,
  // EDIST: no Linux equivalent
  [54] = EAGAIN,		// EWOULDBLOCK
  [AIX_EINPROGRESS] = EINPROGRESS,
  [AIX_EALREADY] = EALREADY,
  [AIX_ENOTSOCK] = ENOTSOCK,
  [AIX_EDESTADDRREQ] = EDESTADDRREQ,
  [AIX_EMSGSIZE] = EMSGSIZE,
  [AIX_EPROTOTYPE] = EPROTOTYPE,
  [AIX_ENOPROTOOPT] = ENOPROTOOPT,
  [AIX_EPROTONOSUPPORT] = EPROTONOSUPPORT,
  [AIX_ESOCKTNOSUPPORT] = ESOCKTNOSUPPORT,
  [AIX_EOPNOTSUPP] = EOPNOTSUPP,
  [AIX_EPFNOSUPPORT] = EPFNOSUPPORT,
  [AIX_EAFNOSUPPORT] = EAFNOSUPPORT,
  [AIX_EADDRINUSE] = EADDRINUSE,
  [AIX_EADDRNOTAVAIL] = EADDRNOTAVAIL,
  [AIX_ENETDOWN] = ENETDOWN,
  [AIX_ENETUNREACH] = ENETUNREACH,
  [AIX_ENETRESET] = ENETRESET,
  [AIX_ECONNABORTED] = ECONNABORTED,
  [AIX_ECONNRESET] = ECONNRESET,
  [AIX_ENOBUFS] = ENOBUFS,
  [AIX_EISCONN] = EISCONN,
  [AIX_ENOTCONN] = ENOTCONN,
  [AIX_ESHUTDOWN] = ESHUTDOWN,
  [AIX_ETIMEDOUT] = ETIMEDOUT,
  [AIX_ECONNREFUSED] = ECONNREFUSED,
  [AIX_EHOSTDOWN] = EHOSTDOWN,
  [AIX_EHOSTUNREACH] = EHOSTUNREACH,
  [AIX_ERESTART] = ERESTART,
  [AIX_EPROCLIM] = EPROCLIM,
  [AIX_EUSERS] = EUSERS,
  [AIX_ELOOP] = ELOOP,
  [AIX_ENAMETOOLONG] = ENAMETOOLONG,
  [87] = ENOTEMPTY,		// ENOTEMPTY
  [AIX_EDQUOT] = EDQUOT,
  [AIX_ECORRUPT] = ECORRUPT,
  [AIX_EREMOTE] = EREMOTE,
  [AIX_ENOSYS] = ENOSYS,
  [AIX_EMEDIA] = EMEDIA,
  [AIX_ESOFT] = ESOFT,
  [AIX_ENOATTR] = ENOATTR,
  [AIX_ESAD] = ESAD,
  // ENOTRUST: no Linux equivalent
  [AIX_ETOOMANYREFS] = ETOOMANYREFS,
  [AIX_EILSEQ] = EILSEQ,
  [AIX_ECANCELED] = ECANCELED,
  [AIX_ENOSR] = ENOSR,
  [AIX_ETIME] = ETIME,
  [AIX_EBADMSG] = EBADMSG,
  [AIX_EPROTO] = EPROTO,
  [AIX_ENODATA] = ENODATA,
  [AIX_ENOSTR] = ENOSTR,
  [AIX_ENOTSUP] = ENOTSUP,
  [AIX_EMULTIHOP] = EMULTIHOP,
  [AIX_ENOLINK] = ENOLINK,
  [AIX_EOVERFLOW] = EOVERFLOW
};


int
__errno_aix_to_linux (int err)
{
  int conv;

  if (err >= 0 && err < (sizeof (mapping) / sizeof (mapping[0]))
      && ((conv = mapping[err]) != 0 || err == 0))
    return conv;

  /* The error value is not known.  Create a special value which can
     be easily recognized as an invalid result.  */
  return 512 + err;
}
