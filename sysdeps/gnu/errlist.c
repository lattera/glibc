/* This file is generated from errno.texi by errlist.awk.  */

#include <errno.h>

const char *_sys_errlist[] =
  {
    [0] = N_("Success"),
#ifdef EPERM
    [EPERM] = N_("Operation not permitted"),
#endif
#ifdef ENOENT
    [ENOENT] = N_("No such file or directory"),
#endif
#ifdef ESRCH
    [ESRCH] = N_("No such process"),
#endif
#ifdef EINTR
    [EINTR] = N_("Interrupted system call"),
#endif
#ifdef EIO
    [EIO] = N_("Input/output error"),
#endif
#ifdef ENXIO
    [ENXIO] = N_("Device not configured"),
#endif
#ifdef E2BIG
    [E2BIG] = N_("Argument list too long"),
#endif
#ifdef ENOEXEC
    [ENOEXEC] = N_("Exec format error"),
#endif
#ifdef EBADF
    [EBADF] = N_("Bad file descriptor"),
#endif
#ifdef ECHILD
    [ECHILD] = N_("No child processes"),
#endif
#ifdef EDEADLK
    [EDEADLK] = N_("Resource deadlock avoided"),
#endif
#ifdef ENOMEM
    [ENOMEM] = N_("Cannot allocate memory"),
#endif
#ifdef EACCES
    [EACCES] = N_("Permission denied"),
#endif
#ifdef EFAULT
    [EFAULT] = N_("Bad address"),
#endif
#ifdef ENOTBLK
    [ENOTBLK] = N_("Block device required"),
#endif
#ifdef EBUSY
    [EBUSY] = N_("Device or resource busy"),
#endif
#ifdef EEXIST
    [EEXIST] = N_("File exists"),
#endif
#ifdef EXDEV
    [EXDEV] = N_("Invalid cross-device link"),
#endif
#ifdef ENODEV
    [ENODEV] = N_("Operation not supported by device"),
#endif
#ifdef ENOTDIR
    [ENOTDIR] = N_("Not a directory"),
#endif
#ifdef EISDIR
    [EISDIR] = N_("Is a directory"),
#endif
#ifdef EINVAL
    [EINVAL] = N_("Invalid argument"),
#endif
#ifdef EMFILE
    [EMFILE] = N_("Too many open files"),
#endif
#ifdef ENFILE
    [ENFILE] = N_("Too many open files in system"),
#endif
#ifdef ENOTTY
    [ENOTTY] = N_("Inappropriate ioctl for device"),
#endif
#ifdef ETXTBSY
    [ETXTBSY] = N_("Text file busy"),
#endif
#ifdef EFBIG
    [EFBIG] = N_("File too large"),
#endif
#ifdef ENOSPC
    [ENOSPC] = N_("No space left on device"),
#endif
#ifdef ESPIPE
    [ESPIPE] = N_("Illegal seek"),
#endif
#ifdef EROFS
    [EROFS] = N_("Read-only file system"),
#endif
#ifdef EMLINK
    [EMLINK] = N_("Too many links"),
#endif
#ifdef EPIPE
    [EPIPE] = N_("Broken pipe"),
#endif
#ifdef EDOM
    [EDOM] = N_("Numerical argument out of domain"),
#endif
#ifdef ERANGE
    [ERANGE] = N_("Numerical result out of range"),
#endif
#ifdef EAGAIN
    [EAGAIN] = N_("Resource temporarily unavailable"),
#endif
#if defined (EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
    [EWOULDBLOCK] = N_("Operation would block"),
#endif
#ifdef EINPROGRESS
    [EINPROGRESS] = N_("Operation now in progress"),
#endif
#ifdef EALREADY
    [EALREADY] = N_("Operation already in progress"),
#endif
#ifdef ENOTSOCK
    [ENOTSOCK] = N_("Socket operation on non-socket"),
#endif
#ifdef EMSGSIZE
    [EMSGSIZE] = N_("Message too long"),
#endif
#ifdef EPROTOTYPE
    [EPROTOTYPE] = N_("Protocol wrong type for socket"),
#endif
#ifdef ENOPROTOOPT
    [ENOPROTOOPT] = N_("Protocol not available"),
#endif
#ifdef EPROTONOSUPPORT
    [EPROTONOSUPPORT] = N_("Protocol not supported"),
#endif
#ifdef ESOCKTNOSUPPORT
    [ESOCKTNOSUPPORT] = N_("Socket type not supported"),
#endif
#ifdef EOPNOTSUPP
    [EOPNOTSUPP] = N_("Operation not supported"),
#endif
#ifdef EPFNOSUPPORT
    [EPFNOSUPPORT] = N_("Protocol family not supported"),
#endif
#ifdef EAFNOSUPPORT
    [EAFNOSUPPORT] = N_("Address family not supported by protocol"),
#endif
#ifdef EADDRINUSE
    [EADDRINUSE] = N_("Address already in use"),
#endif
#ifdef EADDRNOTAVAIL
    [EADDRNOTAVAIL] = N_("Cannot assign requested address"),
#endif
#ifdef ENETDOWN
    [ENETDOWN] = N_("Network is down"),
#endif
#ifdef ENETUNREACH
    [ENETUNREACH] = N_("Network is unreachable"),
#endif
#ifdef ENETRESET
    [ENETRESET] = N_("Network dropped connection on reset"),
#endif
#ifdef ECONNABORTED
    [ECONNABORTED] = N_("Software caused connection abort"),
#endif
#ifdef ECONNRESET
    [ECONNRESET] = N_("Connection reset by peer"),
#endif
#ifdef ENOBUFS
    [ENOBUFS] = N_("No buffer space available"),
#endif
#ifdef EISCONN
    [EISCONN] = N_("Transport endpoint is already connected"),
#endif
#ifdef ENOTCONN
    [ENOTCONN] = N_("Transport endpoint is not connected"),
#endif
#ifdef EDESTADDRREQ
    [EDESTADDRREQ] = N_("Destination address required"),
#endif
#ifdef ESHUTDOWN
    [ESHUTDOWN] = N_("Cannot send after transport endpoint shutdown"),
#endif
#ifdef ETOOMANYREFS
    [ETOOMANYREFS] = N_("Too many references: cannot splice"),
#endif
#ifdef ETIMEDOUT
    [ETIMEDOUT] = N_("Connection timed out"),
#endif
#ifdef ECONNREFUSED
    [ECONNREFUSED] = N_("Connection refused"),
#endif
#ifdef ELOOP
    [ELOOP] = N_("Too many levels of symbolic links"),
#endif
#ifdef ENAMETOOLONG
    [ENAMETOOLONG] = N_("File name too long"),
#endif
#ifdef EHOSTDOWN
    [EHOSTDOWN] = N_("Host is down"),
#endif
#ifdef EHOSTUNREACH
    [EHOSTUNREACH] = N_("No route to host"),
#endif
#ifdef ENOTEMPTY
    [ENOTEMPTY] = N_("Directory not empty"),
#endif
#ifdef EPROCLIM
    [EPROCLIM] = N_("Too many processes"),
#endif
#ifdef EUSERS
    [EUSERS] = N_("Too many users"),
#endif
#ifdef EDQUOT
    [EDQUOT] = N_("Disc quota exceeded"),
#endif
#ifdef ESTALE
    [ESTALE] = N_("Stale NFS file handle"),
#endif
#ifdef EREMOTE
    [EREMOTE] = N_("Object is remote"),
#endif
#ifdef EBADRPC
    [EBADRPC] = N_("RPC struct is bad"),
#endif
#ifdef ERPCMISMATCH
    [ERPCMISMATCH] = N_("RPC version wrong"),
#endif
#ifdef EPROGUNAVAIL
    [EPROGUNAVAIL] = N_("RPC program not available"),
#endif
#ifdef EPROGMISMATCH
    [EPROGMISMATCH] = N_("RPC program version wrong"),
#endif
#ifdef EPROCUNAVAIL
    [EPROCUNAVAIL] = N_("RPC bad procedure for program"),
#endif
#ifdef ENOLCK
    [ENOLCK] = N_("No locks available"),
#endif
#ifdef EFTYPE
    [EFTYPE] = N_("Inappropriate file type or format"),
#endif
#ifdef EAUTH
    [EAUTH] = N_("Authentication error"),
#endif
#ifdef ENEEDAUTH
    [ENEEDAUTH] = N_("Need authenticator"),
#endif
#ifdef ENOSYS
    [ENOSYS] = N_("Function not implemented"),
#endif
#ifdef EILSEQ
    [EILSEQ] = N_("Invalid or incomplete multibyte or wide character"),
#endif
#ifdef EBACKGROUND
    [EBACKGROUND] = N_("Inappropriate operation for background process"),
#endif
#ifdef EDIED
    [EDIED] = N_("Translator died"),
#endif
#ifdef ED
    [ED] = N_("?"),
#endif
#ifdef EGREGIOUS
    [EGREGIOUS] = N_("You really blew it this time"),
#endif
#ifdef EIEIO
    [EIEIO] = N_("Computer bought the farm"),
#endif
#ifdef EGRATUITOUS
    [EGRATUITOUS] = N_("Gratuitous error"),
#endif
#ifdef ERESTART
    [ERESTART] = N_("Interrupted system call should be restarted"),
#endif
#ifdef ENOMSG
    [ENOMSG] = N_("No message of desired type"),
#endif
#ifdef EIDRM
    [EIDRM] = N_("Identifier removed"),
#endif
#ifdef ECHRNG
    [ECHRNG] = N_("Channel number out of range"),
#endif
#ifdef EL2NSYNC
    [EL2NSYNC] = N_("Level 2 not synchronized"),
#endif
#ifdef EL3HLT
    [EL3HLT] = N_("Level 3 halted"),
#endif
#ifdef EL3RST
    [EL3RST] = N_("Level 3 reset"),
#endif
#ifdef ELNRNG
    [ELNRNG] = N_("Link number out of range"),
#endif
#ifdef EUNATCH
    [EUNATCH] = N_("Protocol driver not attached"),
#endif
#ifdef ENOCSI
    [ENOCSI] = N_("No CSI structure available"),
#endif
#ifdef EL2HLT
    [EL2HLT] = N_("Level 2 halted"),
#endif
#ifdef EBADE
    [EBADE] = N_("Invalid exchange"),
#endif
#ifdef EBADR
    [EBADR] = N_("Invalid request descriptor"),
#endif
#ifdef EXFULL
    [EXFULL] = N_("Exchange full"),
#endif
#ifdef ENOANO
    [ENOANO] = N_("No anode"),
#endif
#ifdef EBADRQC
    [EBADRQC] = N_("Invalid request code"),
#endif
#ifdef EBADSLT
    [EBADSLT] = N_("Invalid slot"),
#endif
#ifdef EDEADLOCK
    [EDEADLOCK] = N_("File locking deadlock error"),
#endif
#ifdef EBFONT
    [EBFONT] = N_("Bad font file format"),
#endif
#ifdef ENOSTR
    [ENOSTR] = N_("Device not a stream"),
#endif
#ifdef ENODATA
    [ENODATA] = N_("No data available"),
#endif
#ifdef ETIME
    [ETIME] = N_("Timer expired"),
#endif
#ifdef ENOSR
    [ENOSR] = N_("Out of streams resources"),
#endif
#ifdef ENONET
    [ENONET] = N_("Machine is not on the network"),
#endif
#ifdef ENOPKG
    [ENOPKG] = N_("Package not installed"),
#endif
#ifdef ENOLINK
    [ENOLINK] = N_("Link has been severed"),
#endif
#ifdef EADV
    [EADV] = N_("Advertise error"),
#endif
#ifdef ESRMNT
    [ESRMNT] = N_("Srmount error"),
#endif
#ifdef ECOMM
    [ECOMM] = N_("Communication error on send"),
#endif
#ifdef EPROTO
    [EPROTO] = N_("Protocol error"),
#endif
#ifdef EMULTIHOP
    [EMULTIHOP] = N_("Multihop attempted"),
#endif
#ifdef EDOTDOT
    [EDOTDOT] = N_("RFS specific error"),
#endif
#ifdef EBADMSG
    [EBADMSG] = N_("Not a data message"),
#endif
#ifdef EOVERFLOW
    [EOVERFLOW] = N_("Value too large for defined data type"),
#endif
#ifdef ENOTUNIQ
    [ENOTUNIQ] = N_("Name not unique on network"),
#endif
#ifdef EBADFD
    [EBADFD] = N_("File descriptor in bad state"),
#endif
#ifdef EREMCHG
    [EREMCHG] = N_("Remote address changed"),
#endif
#ifdef ELIBACC
    [ELIBACC] = N_("Can not access a needed shared library"),
#endif
#ifdef ELIBBAD
    [ELIBBAD] = N_("Accessing a corrupted shared library"),
#endif
#ifdef ELIBSCN
    [ELIBSCN] = N_(".lib section in a.out corrupted"),
#endif
#ifdef ELIBMAX
    [ELIBMAX] = N_("Attempting to link in too many shared libraries"),
#endif
#ifdef ELIBEXEC
    [ELIBEXEC] = N_("Cannot exec a shared library directly"),
#endif
#ifdef ESTRPIPE
    [ESTRPIPE] = N_("Streams pipe error"),
#endif
#ifdef EUCLEAN
    [EUCLEAN] = N_("Structure needs cleaning"),
#endif
#ifdef ENOTNAM
    [ENOTNAM] = N_("Not a XENIX named type file"),
#endif
#ifdef ENAVAIL
    [ENAVAIL] = N_("No XENIX semaphores available"),
#endif
#ifdef EISNAM
    [EISNAM] = N_("Is a named type file"),
#endif
#ifdef EREMOTEIO
    [EREMOTEIO] = N_("Remote I/O error"),
#endif
  };

const int _sys_nerr = sizeof _sys_errlist / sizeof _sys_errlist[0];
weak_alias (_sys_errlist, sys_errlist)
weak_alias (_sys_nerr, sys_nerr)
