/* This file is generated from errno.texi by errlist.awk.  */

#ifndef HAVE_GNU_LD
#define _sys_nerr sys_nerr
#define _sys_errlist sys_errlist
#endif

const char *_sys_errlist[] =
  {
    "Success",
    "Operation not permitted",          /* 1 = EPERM */
    "No such file or directory",        /* 2 = ENOENT */
    "No such process",                  /* 3 = ESRCH */
    "Interrupted system call",          /* 4 = EINTR */
    "Input/output error",               /* 5 = EIO */
    "Device not configured",            /* 6 = ENXIO */
    "Argument list too long",           /* 7 = E2BIG */
    "Exec format error",                /* 8 = ENOEXEC */
    "Bad file descriptor",              /* 9 = EBADF */
    "No child processes",               /* 10 = ECHILD */
    "Resource deadlock avoided",        /* 11 = EDEADLK */
    "Cannot allocate memory",           /* 12 = ENOMEM */
    "Permission denied",                /* 13 = EACCES */
    "Bad address",                      /* 14 = EFAULT */
    "Block device required",            /* 15 = ENOTBLK */
    "Device busy",                      /* 16 = EBUSY */
    "File exists",                      /* 17 = EEXIST */
    "Invalid cross-device link",        /* 18 = EXDEV */
    "Operation not supported by device",/* 19 = ENODEV */
    "Not a directory",                  /* 20 = ENOTDIR */
    "Is a directory",                   /* 21 = EISDIR */
    "Invalid argument",                 /* 22 = EINVAL */
    "Too many open files",              /* 23 = EMFILE */
    "Too many open files in system",    /* 24 = ENFILE */
    "Inappropriate ioctl for device",   /* 25 = ENOTTY */
    "Text file busy",                   /* 26 = ETXTBSY */
    "File too large",                   /* 27 = EFBIG */
    "No space left on device",          /* 28 = ENOSPC */
    "Illegal seek",                     /* 29 = ESPIPE */
    "Read-only file system",            /* 30 = EROFS */
    "Too many links",                   /* 31 = EMLINK */
    "Broken pipe",                      /* 32 = EPIPE */
    "Numerical argument out of domain", /* 33 = EDOM */
    "Numerical result out of range",    /* 34 = ERANGE */
    "Operation would block",            /* 35 = EWOULDBLOCK */
    "Operation now in progress",        /* 36 = EINPROGRESS */
    "Operation already in progress",    /* 37 = EALREADY */
    "Socket operation on non-socket",   /* 38 = ENOTSOCK */
    "Destination address required",     /* 39 = EDESTADDRREQ */
    "Message too long",                 /* 40 = EMSGSIZE */
    "Protocol wrong type for socket",   /* 41 = EPROTOTYPE */
    "Protocol not available",           /* 42 = ENOPROTOOPT */
    "Protocol not supported",           /* 43 = EPROTONOSUPPORT */
    "Socket type not supported",        /* 44 = ESOCKTNOSUPPORT */
    "Operation not supported",          /* 45 = EOPNOTSUPP */
    "Protocol family not supported",    /* 46 = EPFNOSUPPORT */
    "Address family not supported by protocol family",/* 47 = EAFNOSUPPORT */
    "Address already in use",           /* 48 = EADDRINUSE */
    "Can't assign requested address",   /* 49 = EADDRNOTAVAIL */
    "Network is down",                  /* 50 = ENETDOWN */
    "Network is unreachable",           /* 51 = ENETUNREACH */
    "Network dropped connection on reset",/* 52 = ENETRESET */
    "Software caused connection abort", /* 53 = ECONNABORTED */
    "Connection reset by peer",         /* 54 = ECONNRESET */
    "No buffer space available",        /* 55 = ENOBUFS */
    "Socket is already connected",      /* 56 = EISCONN */
    "Socket is not connected",          /* 57 = ENOTCONN */
    "Can't send after socket shutdown", /* 58 = ESHUTDOWN */
    "Connection timed out",             /* 59 = ETIMEDOUT */
    "Connection refused",               /* 60 = ECONNREFUSED */
    "Too many levels of symbolic links",/* 61 = ELOOP */
    "File name too long",               /* 62 = ENAMETOOLONG */
    "Host is down",                     /* 63 = EHOSTDOWN */
    "No route to host",                 /* 64 = EHOSTUNREACH */
    "Directory not empty",              /* 65 = ENOTEMPTY */
    "Too many users",                   /* 66 = EUSERS */
    "Disc quota exceeded",              /* 67 = EDQUOT */
    "Stale NFS file handle",            /* 68 = ESTALE */
    "Too many levels of remote in path",/* 69 = EREMOTE */
    "No locks available",               /* 70 = ENOLCK */
    "Function not implemented",         /* 71 = ENOSYS */
    "Inappropriate operation for background process",/* 72 = EBACKGROUND */
    "?",                                /* 73 = ED */
    "You really blew it this time",     /* 74 = EGREGIOUS */
    "Computer bought the farm",         /* 75 = EIEIO */
    "Gratuitous error",                 /* 76 = EGRATUITOUS */
  };

#include <errno.h>
#if _HURD_ERRNOS != 77
#error errlist/errnos generation bug
#endif
const int _sys_nerr = 77;
