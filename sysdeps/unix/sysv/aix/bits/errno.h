/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* This file defines the `errno' constants.  */

#if !defined __Emath_defined && (defined _ERRNO_H || defined __need_Emath)
#undef	__need_Emath
#define	__Emath_defined	1

#endif

#ifdef	_ERRNO_H
# undef EDOM
# undef EILSEQ
# undef ERANGE

# define EPERM		1	/* Operation not permitted.  */
# define ENOENT		2	/* No such file or directory.  */
# define ESRCH		3	/* No such process.  */
# define EINTR		4	/* interrupted system call.  */
# define EIO		5	/* I/O error.  */
# define ENXIO		6	/* No such device or address.  */
# define E2BIG		7	/* Arg list too long.  */
# define ENOEXEC	8	/* Exec format error.  */
# define EBADF		9	/* Bad file descriptor.  */
# define ECHILD		10	/* No child processes.  */
# define EAGAIN		11	/* Resource temporarily unavailable.  */
# define ENOMEM		12	/* Not enough space.  */
# define EACCES		13	/* Permission denied.  */
# define EFAULT		14	/* Bad address.  */
# define ENOTBLK	15	/* Block device required.  */
# define EBUSY		16	/* Resource busy.  */
# define EEXIST		17	/* File exists.  */
# define EXDEV		18	/* Improper link.  */
# define ENODEV		19	/* No such device.  */
# define ENOTDIR	20	/* Not a directory.  */
# define EISDIR		21	/* Is a directory.  */
# define EINVAL		22	/* Invalid argument.  */
# define ENFILE		23	/* Too many open files in system.  */
# define EMFILE		24	/* Too many open files.  */
# define ENOTTY		25	/* Inappropriate I/O control operation.  */
# define ETXTBSY	26	/* Text file busy.  */
# define EFBIG		27	/* File too large.  */
# define ENOSPC		28	/* No space left on device.  */
# define ESPIPE		29	/* Invalid seek.  */
# define EROFS		30	/* Read only file system.  */
# define EMLINK		31	/* Too many links.  */
# define EPIPE		32	/* Broken pipe.  */
# define EDOM		33	/* Domain error within math function.  */
# define ERANGE		34	/* Result too large.  */
# define ENOMSG		35	/* No message of desired type.  */
# define EIDRM		36	/* Identifier removed.  */
# define ECHRNG		37	/* Channel number out of range.  */
# define EL2NSYNC	38	/* Level 2 not synchronized.  */
# define EL3HLT		39	/* Level 3 halted.  */
# define EL3RST		40	/* Level 3 reset.  */
# define ELNRNG		41	/* Link number out of range.  */
# define EUNATCH	42	/* Protocol driver not attached.  */
# define ENOCSI		43	/* No CSI structure available.  */
# define EL2HLT		44	/* Level 2 halted.  */
# define EDEADLK	45	/* Resource deadlock avoided.  */
# define ENOTREADY	46	/* Device not ready.  */
# define EWRPROTECT	47	/* Write-protected media.  */
# define EFORMAT	48	/* Unformatted media.  */
# define ENOLCK		49	/* No locks available.  */
# define ENOCONNECT	50	/* No connection.  */
# define ESTALE		52	/* No filesystem.  */
# define EDIST		53	/* Old, currently unused AIX errno.  */
# if __USE_XOPEN_EXTENDED
#  define EWOULDBLOCK	EAGAIN	/* Operation would block.  */
# else /* __USE_XOPEN_EXTENDED */
#  define EWOULDBLOCK	54
# endif /* __USE_XOPEN_EXTENDED */
# define EINPROGRESS	55	/* Operation now in progress.  */
# define EALREADY	56	/* Operation already in progress.  */
# define ENOTSOCK	57	/* Socket operation on non-socket.  */
# define EDESTADDRREQ	58	/* Destination address required.  */
# define EDESTADDREQ	EDESTADDRREQ /* Destination address required.  */
# define EMSGSIZE	59	/* Message too long.  */
# define EPROTOTYPE	60	/* Protocol wrong type for socket.  */
# define ENOPROTOOPT	61	/* Protocol not available.  */
# define EPROTONOSUPPORT 62	/* Protocol not supported.  */
# define ESOCKTNOSUPPORT 63	/* Socket type not supported.  */
# define EOPNOTSUPP	64	/* Operation not supported on socket.  */
# define EPFNOSUPPORT	65	/* Protocol family not supported.  */
# define EAFNOSUPPORT	66	/* Address family not supported by protocol
				   family.  */
# define EADDRINUSE	67	/* Address already in use.  */
# define EADDRNOTAVAIL	68	/* Can't assign requested address.  */
# define ENETDOWN	69	/* Network is down.  */
# define ENETUNREACH	70	/* Network is unreachable.  */
# define ENETRESET	71	/* Network dropped connection on reset.  */
# define ECONNABORTED	72	/* Software caused connection abort.  */
# define ECONNRESET	73	/* Connection reset by peer.  */
# define ENOBUFS	74	/* No buffer space available.  */
# define EISCONN	75	/* Socket is already connected.  */
# define ENOTCONN	76	/* Socket is not connected.  */
# define ESHUTDOWN	77	/* Can't send after socket shutdown.  */
# define ETIMEDOUT	78	/* Connection timed out.  */
# define ECONNREFUSED	79	/* Connection refused.  */
# define EHOSTDOWN	80	/* Host is down.  */
# define EHOSTUNREACH	81	/* No route to host.  */
# define ERESTART	82	/* Restart the system call.  */
# define EPROCLIM	83	/* Too many processes.  */
# define EUSERS		84	/* Too many users.  */
# define ELOOP		85	/* Too many levels of symbolic links.  */
# define ENAMETOOLONG	86	/* File name too long.  */
# define ENOTEMPTY	EEXIST	/* Directory not empty.  */
# define EDQUOT		88	/* Disc quota exceeded.  */
# define ECORRUPT	89	/* Invalid file system control data.  */
# define EREMOTE	93	/* Item is not local to host.  */
# define ENOSYS		109	/* Function not implemented POSIX.  */
# define EMEDIA		110	/* Media surface error.  */
# define ESOFT		111	/* I/O completed, but needs relocation.  */
# define ENOATTR	112	/* No attribute found.  */
# define ESAD		113	/* Security authentication denied.  */
# define ENOTRUST	114	/* Not a trusted program.  */
# define ETOOMANYREFS	115	/* Too many references: can't splice.  */
# define EILSEQ		116	/* Invalid wide character.  */
# define ECANCELED	117	/* Asynchronous i/o cancelled.  */
# define ENOSR		118	/* Temp out of streams resources.  */
# define ETIME		119	/* I_STR ioctl timed out.  */
# define EBADMSG	120	/* Wrong message type at stream head.  */
# define EPROTO		121	/* STREAMS protocol error.  */
# define ENODATA	122	/* No message ready at stream head.  */
# define ENOSTR		123	/* Fd is not a stream.  */
# define ECLONEME	ERESTART /* This is the way we clone a stream.  */
# define ENOTSUP	124	/* POSIX threads unsupported value.  */
# define EMULTIHOP	125	/* Multihop is not allowed.  */
# define ENOLINK	126	/* The link has been severed.  */
# define EOVERFLOW	127	/* Value too large to be stored in data type.*/

# ifdef _LIBC
#  define __set_errno(val) errno = (val)
# endif
#endif

#if !defined _ERRNO_H && defined __need_Emath
# define EDOM	33	/* Math argument out of domain of function.  */
# define EILSEQ	116	/* Illegal byte sequence.  */
# define ERANGE	34	/* Math result not representable.  */
#endif /* !_ERRNO_H && __need_Emath */
