/* This file is generated from errno.texi by errlist.awk.  */

#include <errno.h>

#ifndef SYS_ERRLIST
# define SYS_ERRLIST _sys_errlist
# define SYS_ERRLIST_ALIAS sys_errlist
#endif
#ifndef SYS_NERR
# define SYS_NERR _sys_nerr
# define SYS_NERR_ALIAS sys_nerr
#endif
#ifndef ERR_REMAP
# define ERR_REMAP(n) n
#endif

const char *const SYS_ERRLIST[] =
  {
    [0] = N_("Success"),
#ifdef EPERM
/*
TRANS Operation not permitted; only the owner of the file (or other resource)
TRANS or processes with special privileges can perform the operation. */
    [ERR_REMAP (EPERM)] = N_("Operation not permitted"),
#endif
#ifdef ENOENT
/*
TRANS No such file or directory.  This is a ``file doesn't exist'' error
TRANS for ordinary files that are referenced in contexts where they are
TRANS expected to already exist. */
    [ERR_REMAP (ENOENT)] = N_("No such file or directory"),
#endif
#ifdef ESRCH
/*
TRANS No process matches the specified process ID. */
    [ERR_REMAP (ESRCH)] = N_("No such process"),
#endif
#ifdef EINTR
/*
TRANS Interrupted function call; an asynchronous signal occurred and prevented
TRANS completion of the call.  When this happens, you should try the call
TRANS again.
TRANS 
TRANS You can choose to have functions resume after a signal that is handled,
TRANS rather than failing with @code{EINTR}; see @ref{Interrupted
TRANS Primitives}. */
    [ERR_REMAP (EINTR)] = N_("Interrupted system call"),
#endif
#ifdef EIO
/*
TRANS Input/output error; usually used for physical read or write errors. */
    [ERR_REMAP (EIO)] = N_("Input/output error"),
#endif
#ifdef ENXIO
/*
TRANS No such device or address.  The system tried to use the device
TRANS represented by a file you specified, and it couldn't find the device.
TRANS This can mean that the device file was installed incorrectly, or that
TRANS the physical device is missing or not correctly attached to the
TRANS computer. */
    [ERR_REMAP (ENXIO)] = N_("Device not configured"),
#endif
#ifdef E2BIG
/*
TRANS Argument list too long; used when the arguments passed to a new program
TRANS being executed with one of the @code{exec} functions (@pxref{Executing a
TRANS File}) occupy too much memory space.  This condition never arises in the
TRANS GNU system. */
    [ERR_REMAP (E2BIG)] = N_("Argument list too long"),
#endif
#ifdef ENOEXEC
/*
TRANS Invalid executable file format.  This condition is detected by the
TRANS @code{exec} functions; see @ref{Executing a File}. */
    [ERR_REMAP (ENOEXEC)] = N_("Exec format error"),
#endif
#ifdef EBADF
/*
TRANS Bad file descriptor; for example, I/O on a descriptor that has been
TRANS closed or reading from a descriptor open only for writing (or vice
TRANS versa). */
    [ERR_REMAP (EBADF)] = N_("Bad file descriptor"),
#endif
#ifdef ECHILD
/*
TRANS There are no child processes.  This error happens on operations that are
TRANS supposed to manipulate child processes, when there aren't any processes
TRANS to manipulate. */
    [ERR_REMAP (ECHILD)] = N_("No child processes"),
#endif
#ifdef EDEADLK
/*
TRANS Deadlock avoided; allocating a system resource would have resulted in a
TRANS deadlock situation.  The system does not guarantee that it will notice
TRANS all such situations.  This error means you got lucky and the system
TRANS noticed; it might just hang.  @xref{File Locks}, for an example. */
    [ERR_REMAP (EDEADLK)] = N_("Resource deadlock avoided"),
#endif
#ifdef ENOMEM
/*
TRANS No memory available.  The system cannot allocate more virtual memory
TRANS because its capacity is full. */
    [ERR_REMAP (ENOMEM)] = N_("Cannot allocate memory"),
#endif
#ifdef EACCES
/*
TRANS Permission denied; the file permissions do not allow the attempted operation. */
    [ERR_REMAP (EACCES)] = N_("Permission denied"),
#endif
#ifdef EFAULT
/*
TRANS Bad address; an invalid pointer was detected.
TRANS In the GNU system, this error never happens; you get a signal instead. */
    [ERR_REMAP (EFAULT)] = N_("Bad address"),
#endif
#ifdef ENOTBLK
/*
TRANS A file that isn't a block special file was given in a situation that
TRANS requires one.  For example, trying to mount an ordinary file as a file
TRANS system in Unix gives this error. */
    [ERR_REMAP (ENOTBLK)] = N_("Block device required"),
#endif
#ifdef EBUSY
/*
TRANS Resource busy; a system resource that can't be shared is already in use.
TRANS For example, if you try to delete a file that is the root of a currently
TRANS mounted filesystem, you get this error. */
    [ERR_REMAP (EBUSY)] = N_("Device or resource busy"),
#endif
#ifdef EEXIST
/*
TRANS File exists; an existing file was specified in a context where it only
TRANS makes sense to specify a new file. */
    [ERR_REMAP (EEXIST)] = N_("File exists"),
#endif
#ifdef EXDEV
/*
TRANS An attempt to make an improper link across file systems was detected.
TRANS This happens not only when you use @code{link} (@pxref{Hard Links}) but
TRANS also when you rename a file with @code{rename} (@pxref{Renaming Files}). */
    [ERR_REMAP (EXDEV)] = N_("Invalid cross-device link"),
#endif
#ifdef ENODEV
/*
TRANS The wrong type of device was given to a function that expects a
TRANS particular sort of device. */
    [ERR_REMAP (ENODEV)] = N_("No such device"),
#endif
#ifdef ENOTDIR
/*
TRANS A file that isn't a directory was specified when a directory is required. */
    [ERR_REMAP (ENOTDIR)] = N_("Not a directory"),
#endif
#ifdef EISDIR
/*
TRANS File is a directory; you cannot open a directory for writing,
TRANS or create or remove hard links to it. */
    [ERR_REMAP (EISDIR)] = N_("Is a directory"),
#endif
#ifdef EINVAL
/*
TRANS Invalid argument.  This is used to indicate various kinds of problems
TRANS with passing the wrong argument to a library function. */
    [ERR_REMAP (EINVAL)] = N_("Invalid argument"),
#endif
#ifdef EMFILE
/*
TRANS The current process has too many files open and can't open any more.
TRANS Duplicate descriptors do count toward this limit.
TRANS 
TRANS In BSD and GNU, the number of open files is controlled by a resource
TRANS limit that can usually be increased.  If you get this error, you might
TRANS want to increase the @code{RLIMIT_NOFILE} limit or make it unlimited;
TRANS @pxref{Limits on Resources}. */
    [ERR_REMAP (EMFILE)] = N_("Too many open files"),
#endif
#ifdef ENFILE
/*
TRANS There are too many distinct file openings in the entire system.  Note
TRANS that any number of linked channels count as just one file opening; see
TRANS @ref{Linked Channels}.  This error never occurs in the GNU system. */
    [ERR_REMAP (ENFILE)] = N_("Too many open files in system"),
#endif
#ifdef ENOTTY
/*
TRANS Inappropriate I/O control operation, such as trying to set terminal
TRANS modes on an ordinary file. */
    [ERR_REMAP (ENOTTY)] = N_("Inappropriate ioctl for device"),
#endif
#ifdef ETXTBSY
/*
TRANS An attempt to execute a file that is currently open for writing, or
TRANS write to a file that is currently being executed.  Often using a
TRANS debugger to run a program is considered having it open for writing and
TRANS will cause this error.  (The name stands for ``text file busy''.)  This
TRANS is not an error in the GNU system; the text is copied as necessary. */
    [ERR_REMAP (ETXTBSY)] = N_("Text file busy"),
#endif
#ifdef EFBIG
/*
TRANS File too big; the size of a file would be larger than allowed by the system. */
    [ERR_REMAP (EFBIG)] = N_("File too large"),
#endif
#ifdef ENOSPC
/*
TRANS No space left on device; write operation on a file failed because the
TRANS disk is full. */
    [ERR_REMAP (ENOSPC)] = N_("No space left on device"),
#endif
#ifdef ESPIPE
/*
TRANS Invalid seek operation (such as on a pipe). */
    [ERR_REMAP (ESPIPE)] = N_("Illegal seek"),
#endif
#ifdef EROFS
/*
TRANS An attempt was made to modify something on a read-only file system. */
    [ERR_REMAP (EROFS)] = N_("Read-only file system"),
#endif
#ifdef EMLINK
/*
TRANS Too many links; the link count of a single file would become too large.
TRANS @code{rename} can cause this error if the file being renamed already has
TRANS as many links as it can take (@pxref{Renaming Files}). */
    [ERR_REMAP (EMLINK)] = N_("Too many links"),
#endif
#ifdef EPIPE
/*
TRANS Broken pipe; there is no process reading from the other end of a pipe.
TRANS Every library function that returns this error code also generates a
TRANS @code{SIGPIPE} signal; this signal terminates the program if not handled
TRANS or blocked.  Thus, your program will never actually see @code{EPIPE}
TRANS unless it has handled or blocked @code{SIGPIPE}. */
    [ERR_REMAP (EPIPE)] = N_("Broken pipe"),
#endif
#ifdef EDOM
/*
TRANS Domain error; used by mathematical functions when an argument value does
TRANS not fall into the domain over which the function is defined. */
    [ERR_REMAP (EDOM)] = N_("Numerical argument out of domain"),
#endif
#ifdef ERANGE
/*
TRANS Range error; used by mathematical functions when the result value is
TRANS not representable because of overflow or underflow. */
    [ERR_REMAP (ERANGE)] = N_("Numerical result out of range"),
#endif
#ifdef EAGAIN
/*
TRANS Resource temporarily unavailable; the call might work if you try again
TRANS later.  The macro @code{EWOULDBLOCK} is another name for @code{EAGAIN};
TRANS they are always the same in the GNU C library.
TRANS 
TRANS This error can happen in a few different situations:
TRANS 
TRANS @itemize @bullet
TRANS @item
TRANS An operation that would block was attempted on an object that has
TRANS non-blocking mode selected.  Trying the same operation again will block
TRANS until some external condition makes it possible to read, write, or
TRANS connect (whatever the operation).  You can use @code{select} to find out
TRANS when the operation will be possible; @pxref{Waiting for I/O}.
TRANS 
TRANS @strong{Portability Note:} In many older Unix systems, this condition
TRANS was indicated by @code{EWOULDBLOCK}, which was a distinct error code
TRANS different from @code{EAGAIN}.  To make your program portable, you should
TRANS check for both codes and treat them the same.
TRANS 
TRANS @item
TRANS A temporary resource shortage made an operation impossible.  @code{fork}
TRANS can return this error.  It indicates that the shortage is expected to
TRANS pass, so your program can try the call again later and it may succeed.
TRANS It is probably a good idea to delay for a few seconds before trying it
TRANS again, to allow time for other processes to release scarce resources.
TRANS Such shortages are usually fairly serious and affect the whole system,
TRANS so usually an interactive program should report the error to the user
TRANS and return to its command loop.
TRANS @end itemize */
    [ERR_REMAP (EAGAIN)] = N_("Resource temporarily unavailable"),
#endif
#if defined (EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
/*
TRANS In the GNU C library, this is another name for @code{EAGAIN} (above).
TRANS The values are always the same, on every operating system.
TRANS 
TRANS C libraries in many older Unix systems have @code{EWOULDBLOCK} as a
TRANS separate error code. */
    [ERR_REMAP (EWOULDBLOCK)] = N_("Operation would block"),
#endif
#ifdef EINPROGRESS
/*
TRANS An operation that cannot complete immediately was initiated on an object
TRANS that has non-blocking mode selected.  Some functions that must always
TRANS block (such as @code{connect}; @pxref{Connecting}) never return
TRANS @code{EAGAIN}.  Instead, they return @code{EINPROGRESS} to indicate that
TRANS the operation has begun and will take some time.  Attempts to manipulate
TRANS the object before the call completes return @code{EALREADY}.  You can
TRANS use the @code{select} function to find out when the pending operation
TRANS has completed; @pxref{Waiting for I/O}. */
    [ERR_REMAP (EINPROGRESS)] = N_("Operation now in progress"),
#endif
#ifdef EALREADY
/*
TRANS An operation is already in progress on an object that has non-blocking
TRANS mode selected. */
    [ERR_REMAP (EALREADY)] = N_("Operation already in progress"),
#endif
#ifdef ENOTSOCK
/*
TRANS A file that isn't a socket was specified when a socket is required. */
    [ERR_REMAP (ENOTSOCK)] = N_("Socket operation on non-socket"),
#endif
#ifdef EMSGSIZE
/*
TRANS The size of a message sent on a socket was larger than the supported
TRANS maximum size. */
    [ERR_REMAP (EMSGSIZE)] = N_("Message too long"),
#endif
#ifdef EPROTOTYPE
/*
TRANS The socket type does not support the requested communications protocol. */
    [ERR_REMAP (EPROTOTYPE)] = N_("Protocol wrong type for socket"),
#endif
#ifdef ENOPROTOOPT
/*
TRANS You specified a socket option that doesn't make sense for the
TRANS particular protocol being used by the socket.  @xref{Socket Options}. */
    [ERR_REMAP (ENOPROTOOPT)] = N_("Protocol not available"),
#endif
#ifdef EPROTONOSUPPORT
/*
TRANS The socket domain does not support the requested communications protocol
TRANS (perhaps because the requested protocol is completely invalid).
TRANS @xref{Creating a Socket}. */
    [ERR_REMAP (EPROTONOSUPPORT)] = N_("Protocol not supported"),
#endif
#ifdef ESOCKTNOSUPPORT
/*
TRANS The socket type is not supported. */
    [ERR_REMAP (ESOCKTNOSUPPORT)] = N_("Socket type not supported"),
#endif
#ifdef EOPNOTSUPP
/*
TRANS The operation you requested is not supported.  Some socket functions
TRANS don't make sense for all types of sockets, and others may not be
TRANS implemented for all communications protocols.  In the GNU system, this
TRANS error can happen for many calls when the object does not support the
TRANS particular operation; it is a generic indication that the server knows
TRANS nothing to do for that call. */
    [ERR_REMAP (EOPNOTSUPP)] = N_("Operation not supported"),
#endif
#ifdef EPFNOSUPPORT
/*
TRANS The socket communications protocol family you requested is not supported. */
    [ERR_REMAP (EPFNOSUPPORT)] = N_("Protocol family not supported"),
#endif
#ifdef EAFNOSUPPORT
/*
TRANS The address family specified for a socket is not supported; it is
TRANS inconsistent with the protocol being used on the socket.  @xref{Sockets}. */
    [ERR_REMAP (EAFNOSUPPORT)] = N_("Address family not supported by protocol"),
#endif
#ifdef EADDRINUSE
/*
TRANS The requested socket address is already in use.  @xref{Socket Addresses}. */
    [ERR_REMAP (EADDRINUSE)] = N_("Address already in use"),
#endif
#ifdef EADDRNOTAVAIL
/*
TRANS The requested socket address is not available; for example, you tried
TRANS to give a socket a name that doesn't match the local host name.
TRANS @xref{Socket Addresses}. */
    [ERR_REMAP (EADDRNOTAVAIL)] = N_("Cannot assign requested address"),
#endif
#ifdef ENETDOWN
/*
TRANS A socket operation failed because the network was down. */
    [ERR_REMAP (ENETDOWN)] = N_("Network is down"),
#endif
#ifdef ENETUNREACH
/*
TRANS A socket operation failed because the subnet containing the remote host
TRANS was unreachable. */
    [ERR_REMAP (ENETUNREACH)] = N_("Network is unreachable"),
#endif
#ifdef ENETRESET
/*
TRANS A network connection was reset because the remote host crashed. */
    [ERR_REMAP (ENETRESET)] = N_("Network dropped connection on reset"),
#endif
#ifdef ECONNABORTED
/*
TRANS A network connection was aborted locally. */
    [ERR_REMAP (ECONNABORTED)] = N_("Software caused connection abort"),
#endif
#ifdef ECONNRESET
/*
TRANS A network connection was closed for reasons outside the control of the
TRANS local host, such as by the remote machine rebooting or an unrecoverable
TRANS protocol violation. */
    [ERR_REMAP (ECONNRESET)] = N_("Connection reset by peer"),
#endif
#ifdef ENOBUFS
/*
TRANS The kernel's buffers for I/O operations are all in use.  In GNU, this
TRANS error is always synonymous with @code{ENOMEM}; you may get one or the
TRANS other from network operations. */
    [ERR_REMAP (ENOBUFS)] = N_("No buffer space available"),
#endif
#ifdef EISCONN
/*
TRANS You tried to connect a socket that is already connected.
TRANS @xref{Connecting}. */
    [ERR_REMAP (EISCONN)] = N_("Transport endpoint is already connected"),
#endif
#ifdef ENOTCONN
/*
TRANS The socket is not connected to anything.  You get this error when you
TRANS try to transmit data over a socket, without first specifying a
TRANS destination for the data.  For a connectionless socket (for datagram
TRANS protocols, such as UDP), you get @code{EDESTADDRREQ} instead. */
    [ERR_REMAP (ENOTCONN)] = N_("Transport endpoint is not connected"),
#endif
#ifdef EDESTADDRREQ
/*
TRANS No default destination address was set for the socket.  You get this
TRANS error when you try to transmit data over a connectionless socket,
TRANS without first specifying a destination for the data with @code{connect}. */
    [ERR_REMAP (EDESTADDRREQ)] = N_("Destination address required"),
#endif
#ifdef ESHUTDOWN
/*
TRANS The socket has already been shut down. */
    [ERR_REMAP (ESHUTDOWN)] = N_("Cannot send after transport endpoint shutdown"),
#endif
#ifdef ETOOMANYREFS
/*
TRANS ??? */
    [ERR_REMAP (ETOOMANYREFS)] = N_("Too many references: cannot splice"),
#endif
#ifdef ETIMEDOUT
/*
TRANS A socket operation with a specified timeout received no response during
TRANS the timeout period. */
    [ERR_REMAP (ETIMEDOUT)] = N_("Connection timed out"),
#endif
#ifdef ECONNREFUSED
/*
TRANS A remote host refused to allow the network connection (typically because
TRANS it is not running the requested service). */
    [ERR_REMAP (ECONNREFUSED)] = N_("Connection refused"),
#endif
#ifdef ELOOP
/*
TRANS Too many levels of symbolic links were encountered in looking up a file name.
TRANS This often indicates a cycle of symbolic links. */
    [ERR_REMAP (ELOOP)] = N_("Too many levels of symbolic links"),
#endif
#ifdef ENAMETOOLONG
/*
TRANS Filename too long (longer than @code{PATH_MAX}; @pxref{Limits for
TRANS Files}) or host name too long (in @code{gethostname} or
TRANS @code{sethostname}; @pxref{Host Identification}). */
    [ERR_REMAP (ENAMETOOLONG)] = N_("File name too long"),
#endif
#ifdef EHOSTDOWN
/*
TRANS The remote host for a requested network connection is down. */
    [ERR_REMAP (EHOSTDOWN)] = N_("Host is down"),
#endif
#ifdef EHOSTUNREACH
/*
TRANS The remote host for a requested network connection is not reachable. */
    [ERR_REMAP (EHOSTUNREACH)] = N_("No route to host"),
#endif
#ifdef ENOTEMPTY
/*
TRANS Directory not empty, where an empty directory was expected.  Typically,
TRANS this error occurs when you are trying to delete a directory. */
    [ERR_REMAP (ENOTEMPTY)] = N_("Directory not empty"),
#endif
#ifdef EPROCLIM
/*
TRANS This means that the per-user limit on new process would be exceeded by
TRANS an attempted @code{fork}.  @xref{Limits on Resources}, for details on
TRANS the @code{RLIMIT_NPROC} limit. */
    [ERR_REMAP (EPROCLIM)] = N_("Too many processes"),
#endif
#ifdef EUSERS
/*
TRANS The file quota system is confused because there are too many users.
TRANS @c This can probably happen in a GNU system when using NFS. */
    [ERR_REMAP (EUSERS)] = N_("Too many users"),
#endif
#ifdef EDQUOT
/*
TRANS The user's disk quota was exceeded. */
    [ERR_REMAP (EDQUOT)] = N_("Disc quota exceeded"),
#endif
#ifdef ESTALE
/*
TRANS Stale NFS file handle.  This indicates an internal confusion in the NFS
TRANS system which is due to file system rearrangements on the server host.
TRANS Repairing this condition usually requires unmounting and remounting
TRANS the NFS file system on the local host. */
    [ERR_REMAP (ESTALE)] = N_("Stale NFS file handle"),
#endif
#ifdef EREMOTE
/*
TRANS An attempt was made to NFS-mount a remote file system with a file name that
TRANS already specifies an NFS-mounted file.
TRANS (This is an error on some operating systems, but we expect it to work
TRANS properly on the GNU system, making this error code impossible.) */
    [ERR_REMAP (EREMOTE)] = N_("Object is remote"),
#endif
#ifdef EBADRPC
/*
TRANS ??? */
    [ERR_REMAP (EBADRPC)] = N_("RPC struct is bad"),
#endif
#ifdef ERPCMISMATCH
/*
TRANS ??? */
    [ERR_REMAP (ERPCMISMATCH)] = N_("RPC version wrong"),
#endif
#ifdef EPROGUNAVAIL
/*
TRANS ??? */
    [ERR_REMAP (EPROGUNAVAIL)] = N_("RPC program not available"),
#endif
#ifdef EPROGMISMATCH
/*
TRANS ??? */
    [ERR_REMAP (EPROGMISMATCH)] = N_("RPC program version wrong"),
#endif
#ifdef EPROCUNAVAIL
/*
TRANS ??? */
    [ERR_REMAP (EPROCUNAVAIL)] = N_("RPC bad procedure for program"),
#endif
#ifdef ENOLCK
/*
TRANS No locks available.  This is used by the file locking facilities; see
TRANS @ref{File Locks}.  This error is never generated by the GNU system, but
TRANS it can result from an operation to an NFS server running another
TRANS operating system. */
    [ERR_REMAP (ENOLCK)] = N_("No locks available"),
#endif
#ifdef EFTYPE
/*
TRANS Inappropriate file type or format.  The file was the wrong type for the
TRANS operation, or a data file had the wrong format.
TRANS 
TRANS On some systems @code{chmod} returns this error if you try to set the
TRANS sticky bit on a non-directory file; @pxref{Setting Permissions}. */
    [ERR_REMAP (EFTYPE)] = N_("Inappropriate file type or format"),
#endif
#ifdef EAUTH
/*
TRANS ??? */
    [ERR_REMAP (EAUTH)] = N_("Authentication error"),
#endif
#ifdef ENEEDAUTH
/*
TRANS ??? */
    [ERR_REMAP (ENEEDAUTH)] = N_("Need authenticator"),
#endif
#ifdef ENOSYS
/*
TRANS Function not implemented.  Some functions have commands or options defined
TRANS that might not be supported in all implementations, and this is the kind
TRANS of error you get if you request them and they are not supported. */
    [ERR_REMAP (ENOSYS)] = N_("Function not implemented"),
#endif
#ifdef EILSEQ
/*
TRANS While decoding a multibyte character the function came along an invalid
TRANS or an incomplete sequence of bytes or the given wide character is invalid. */
    [ERR_REMAP (EILSEQ)] = N_("Invalid or incomplete multibyte or wide character"),
#endif
#ifdef EBACKGROUND
/*
TRANS In the GNU system, servers supporting the @code{term} protocol return
TRANS this error for certain operations when the caller is not in the
TRANS foreground process group of the terminal.  Users do not usually see this
TRANS error because functions such as @code{read} and @code{write} translate
TRANS it into a @code{SIGTTIN} or @code{SIGTTOU} signal.  @xref{Job Control},
TRANS for information on process groups and these signals. */
    [ERR_REMAP (EBACKGROUND)] = N_("Inappropriate operation for background process"),
#endif
#ifdef EDIED
/*
TRANS In the GNU system, opening a file returns this error when the file is
TRANS translated by a program and the translator program dies while starting
TRANS up, before it has connected to the file. */
    [ERR_REMAP (EDIED)] = N_("Translator died"),
#endif
#ifdef ED
/*
TRANS The experienced user will know what is wrong.
TRANS @c This error code is a joke.  Its perror text is part of the joke.
TRANS @c Don't change it. */
    [ERR_REMAP (ED)] = N_("?"),
#endif
#ifdef EGREGIOUS
/*
TRANS You did @strong{what}? */
    [ERR_REMAP (EGREGIOUS)] = N_("You really blew it this time"),
#endif
#ifdef EIEIO
/*
TRANS Go home and have a glass of warm, dairy-fresh milk. */
    [ERR_REMAP (EIEIO)] = N_("Computer bought the farm"),
#endif
#ifdef EGRATUITOUS
/*
TRANS This error code has no purpose. */
    [ERR_REMAP (EGRATUITOUS)] = N_("Gratuitous error"),
#endif
#ifdef EBADMSG
/* */
    [ERR_REMAP (EBADMSG)] = N_("Bad message"),
#endif
#ifdef EIDRM
/* */
    [ERR_REMAP (EIDRM)] = N_("Identifier removed"),
#endif
#ifdef EMULTIHOP
/* */
    [ERR_REMAP (EMULTIHOP)] = N_("Multihop attempted"),
#endif
#ifdef ENODATA
/* */
    [ERR_REMAP (ENODATA)] = N_("No data available"),
#endif
#ifdef ENOLINK
/* */
    [ERR_REMAP (ENOLINK)] = N_("Link has been severed"),
#endif
#ifdef ENOMSG
/* */
    [ERR_REMAP (ENOMSG)] = N_("No message of desired type"),
#endif
#ifdef ENOSR
/* */
    [ERR_REMAP (ENOSR)] = N_("Out of streams resources"),
#endif
#ifdef ENOSTR
/* */
    [ERR_REMAP (ENOSTR)] = N_("Device not a stream"),
#endif
#ifdef EOVERFLOW
/* */
    [ERR_REMAP (EOVERFLOW)] = N_("Value too large for defined data type"),
#endif
#ifdef EPROTO
/* */
    [ERR_REMAP (EPROTO)] = N_("Protocol error"),
#endif
#ifdef ETIME
/* */
    [ERR_REMAP (ETIME)] = N_("Timer expired"),
#endif
#ifdef ERESTART
/* */
    [ERR_REMAP (ERESTART)] = N_("Interrupted system call should be restarted"),
#endif
#ifdef ECHRNG
/* */
    [ERR_REMAP (ECHRNG)] = N_("Channel number out of range"),
#endif
#ifdef EL2NSYNC
/* */
    [ERR_REMAP (EL2NSYNC)] = N_("Level 2 not synchronized"),
#endif
#ifdef EL3HLT
/* */
    [ERR_REMAP (EL3HLT)] = N_("Level 3 halted"),
#endif
#ifdef EL3RST
/* */
    [ERR_REMAP (EL3RST)] = N_("Level 3 reset"),
#endif
#ifdef ELNRNG
/* */
    [ERR_REMAP (ELNRNG)] = N_("Link number out of range"),
#endif
#ifdef EUNATCH
/* */
    [ERR_REMAP (EUNATCH)] = N_("Protocol driver not attached"),
#endif
#ifdef ENOCSI
/* */
    [ERR_REMAP (ENOCSI)] = N_("No CSI structure available"),
#endif
#ifdef EL2HLT
/* */
    [ERR_REMAP (EL2HLT)] = N_("Level 2 halted"),
#endif
#ifdef EBADE
/* */
    [ERR_REMAP (EBADE)] = N_("Invalid exchange"),
#endif
#ifdef EBADR
/* */
    [ERR_REMAP (EBADR)] = N_("Invalid request descriptor"),
#endif
#ifdef EXFULL
/* */
    [ERR_REMAP (EXFULL)] = N_("Exchange full"),
#endif
#ifdef ENOANO
/* */
    [ERR_REMAP (ENOANO)] = N_("No anode"),
#endif
#ifdef EBADRQC
/* */
    [ERR_REMAP (EBADRQC)] = N_("Invalid request code"),
#endif
#ifdef EBADSLT
/* */
    [ERR_REMAP (EBADSLT)] = N_("Invalid slot"),
#endif
#if defined (EDEADLOCK) && EDEADLOCK != EDEADLK
/* */
    [ERR_REMAP (EDEADLOCK)] = N_("File locking deadlock error"),
#endif
#ifdef EBFONT
/* */
    [ERR_REMAP (EBFONT)] = N_("Bad font file format"),
#endif
#ifdef ENONET
/* */
    [ERR_REMAP (ENONET)] = N_("Machine is not on the network"),
#endif
#ifdef ENOPKG
/* */
    [ERR_REMAP (ENOPKG)] = N_("Package not installed"),
#endif
#ifdef EADV
/* */
    [ERR_REMAP (EADV)] = N_("Advertise error"),
#endif
#ifdef ESRMNT
/* */
    [ERR_REMAP (ESRMNT)] = N_("Srmount error"),
#endif
#ifdef ECOMM
/* */
    [ERR_REMAP (ECOMM)] = N_("Communication error on send"),
#endif
#ifdef EDOTDOT
/* */
    [ERR_REMAP (EDOTDOT)] = N_("RFS specific error"),
#endif
#ifdef ENOTUNIQ
/* */
    [ERR_REMAP (ENOTUNIQ)] = N_("Name not unique on network"),
#endif
#ifdef EBADFD
/* */
    [ERR_REMAP (EBADFD)] = N_("File descriptor in bad state"),
#endif
#ifdef EREMCHG
/* */
    [ERR_REMAP (EREMCHG)] = N_("Remote address changed"),
#endif
#ifdef ELIBACC
/* */
    [ERR_REMAP (ELIBACC)] = N_("Can not access a needed shared library"),
#endif
#ifdef ELIBBAD
/* */
    [ERR_REMAP (ELIBBAD)] = N_("Accessing a corrupted shared library"),
#endif
#ifdef ELIBSCN
/* */
    [ERR_REMAP (ELIBSCN)] = N_(".lib section in a.out corrupted"),
#endif
#ifdef ELIBMAX
/* */
    [ERR_REMAP (ELIBMAX)] = N_("Attempting to link in too many shared libraries"),
#endif
#ifdef ELIBEXEC
/* */
    [ERR_REMAP (ELIBEXEC)] = N_("Cannot exec a shared library directly"),
#endif
#ifdef ESTRPIPE
/* */
    [ERR_REMAP (ESTRPIPE)] = N_("Streams pipe error"),
#endif
#ifdef EUCLEAN
/* */
    [ERR_REMAP (EUCLEAN)] = N_("Structure needs cleaning"),
#endif
#ifdef ENOTNAM
/* */
    [ERR_REMAP (ENOTNAM)] = N_("Not a XENIX named type file"),
#endif
#ifdef ENAVAIL
/* */
    [ERR_REMAP (ENAVAIL)] = N_("No XENIX semaphores available"),
#endif
#ifdef EISNAM
/* */
    [ERR_REMAP (EISNAM)] = N_("Is a named type file"),
#endif
#ifdef EREMOTEIO
/* */
    [ERR_REMAP (EREMOTEIO)] = N_("Remote I/O error"),
#endif
#ifdef ENOMEDIUM
/* */
    [ERR_REMAP (ENOMEDIUM)] = N_("No medium found"),
#endif
#ifdef EMEDIUMTYPE
/* */
    [ERR_REMAP (EMEDIUMTYPE)] = N_("Wrong medium type"),
#endif
  };

const int SYS_NERR = sizeof SYS_ERRLIST / sizeof SYS_ERRLIST [0];
#ifdef SYS_ERRLIST_ALIAS
weak_alias (_sys_errlist, SYS_ERRLIST_ALIAS)
#endif
#ifdef SYS_NERR_ALIAS
weak_alias (_sys_nerr, SYS_NERR_ALIAS)
#endif
