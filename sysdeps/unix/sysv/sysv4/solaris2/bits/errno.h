/* Copyright (C) 1991, 1994, 1996, 1997, 1998 Free Software Foundation, Inc.
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

# define EDOM	33	/* Math argument out of domain of function.  */
# define EILSEQ	88	/* Illegal byte sequence.  */
# define ERANGE	34	/* Math result not representable.  */
#endif

#ifdef	_ERRNO_H
/* All error codes the system knows about.  */

# define EPERM		1	/* Not super-user.  */
# define ENOENT		2	/* No such file or directory.  */
# define ESRCH		3	/* No such process.  */
# define EINTR		4	/* Interrupted system call.  */
# define EIO		5	/* I/O error.  */
# define ENXIO		6	/* No such device or address.  */
# define E2BIG		7	/* Arg list too long.  */
# define ENOEXEC	8	/* Exec format error.  */
# define EBADF		9	/* Bad file number.  */
# define ECHILD		10	/* No children.  */
# define EAGAIN		11	/* Resource temporarily unavailable.  */
# define EWOULDBLOCK	EAGAIN
# define ENOMEM		12	/* Not enough core.  */
# define EACCES		13	/* Permission denied.  */
# define EFAULT		14	/* Bad address.  */
# define ENOTBLK	15	/* Block device required.  */
# define EBUSY		16	/* Mount device busy.  */
# define EEXIST		17	/* File exists.  */
# define EXDEV		18	/* Cross-device link.  */
# define ENODEV		19	/* No such device.  */
# define ENOTDIR	20	/* Not a directory.  */
# define EISDIR		21	/* Is a directory.  */
# define EINVAL		22	/* Invalid argument.  */
# define ENFILE		23	/* File table overflow.  */
# define EMFILE		24	/* Too many open files.  */
# define ENOTTY		25	/* Inappropriate ioctl for device.  */
# define ETXTBSY	26	/* Text file busy.  */
# define EFBIG		27	/* File too large.  */
# define ENOSPC		28	/* No space left on device.  */
# define ESPIPE		29	/* Illegal seek.  */
# define EROFS		30	/* Read only file system.  */
# define EMLINK		31	/* Too many links.  */
# define EPIPE		32	/* Broken pipe.  */
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
# define EDEADLK	45	/* Deadlock condition.  */
# define ENOLCK		46	/* No record locks available.  */
# define ECANCELED	47	/* Operation canceled.  */
# define ENOTSUP	48	/* Operation not supported.  */

/* Filesystem Quotas.  */
# define EDQUOT		49	/* Disc quota exceeded.  */

/* Convergent Error Returns.  */
# define EBADE		50	/* Invalid exchange.  */
# define EBADR		51	/* Invalid request descriptor.  */
# define EXFULL		52	/* Exchange full.  */
# define ENOANO		53	/* No anode.  */
# define EBADRQC	54	/* Invalid request code.  */
# define EBADSLT	55	/* Invalid slot.  */
# define EDEADLOCK	56	/* File locking deadlock error.  */

# define EBFONT		57	/* Bad font file fmt.  */

/* STREAM problems.  */
# define ENOSTR		60	/* Device not a stream.  */
# define ENODATA	61	/* No data (for no delay io).  */
# define ETIME		62	/* Timer expired.  */
# define ENOSR		63	/* Out of streams resources.  */

# define ENONET		64	/* Machine is not on the network.  */
# define ENOPKG		65	/* Package not installed.  */
# define EREMOTE	66	/* The object is remote.  */
# define ENOLINK	67	/* The link has been severed.  */
# define EADV		68	/* Advertise error.  */
# define ESRMNT		69	/* Srmount error.  */

# define ECOMM		70	/* Communication error on send.  */
# define EPROTO		71	/* Protocol error.  */
# define EMULTIHOP	74	/* Multihop attempted.  */
# define EBADMSG	77	/* Trying to read unreadable message.  */
# define ENAMETOOLONG	78	/* Path name is too long.  */
# define EOVERFLOW	79	/* Value too large to be stored in data type.*/
# define ENOTUNIQ	80	/* Given log. name not unique.  */
# define EBADFD		81	/* F.d. invalid for this operation.  */
# define EREMCHG	82	/* Remote address changed.  */

/* Shared library problems.  */
# define ELIBACC	83	/* Can't access a needed shared lib.  */
# define ELIBBAD	84	/* Accessing a corrupted shared lib.  */
# define ELIBSCN	85	/* .lib section in a.out corrupted.  */
# define ELIBMAX	86	/* Attempting to link in too many libs.  */
# define ELIBEXEC	87	/* Attempting to exec a shared library.  */
# define ENOSYS		89	/* Unsupported file system operation.  */
# define ELOOP		90	/* Symbolic link loop.  */
# define ERESTART	91	/* Restartable system call.  */
# define ESTRPIPE	92	/* If pipe/FIFO, don't sleep in stream head. */
# define ENOTEMPTY	93	/* Directory not empty.  */
# define EUSERS		94	/* Too many users (for UFS).  */

/* BSD Networking Software: argument errors.  */
# define ENOTSOCK	95	/* Socket operation on non-socket.  */
# define EDESTADDRREQ	96	/* Destination address required.  */
# define EMSGSIZE	97	/* Message too long.  */
# define EPROTOTYPE	98	/* Protocol wrong type for socket.  */
# define ENOPROTOOPT	99	/* Protocol not available.  */
# define EPROTONOSUPPORT 120	/* Protocol not supported.  */
# define ESOCKTNOSUPPORT 121	/* Socket type not supported.  */
# define EOPNOTSUPP	122	/* Operation not supported on socket.  */
# define EPFNOSUPPORT	123	/* Protocol family not supported.  */
# define EAFNOSUPPORT	124	/* Address family not supported by
				   protocol family.  */
# define EADDRINUSE	125	/* Address already in use.  */
# define EADDRNOTAVAIL	126	/* Can't assign requested address.  */
/* BSD Networking Software: operational errors.  */
# define ENETDOWN	127	/* Network is down.  */
# define ENETUNREACH	128	/* Network is unreachable.  */
# define ENETRESET	129	/* Network dropped connection because
				   of reset.  */
# define ECONNABORTED	130	/* Software caused connection abort.  */
# define ECONNRESET	131	/* Connection reset by peer.  */
# define ENOBUFS	132	/* No buffer space available.  */
# define EISCONN	133	/* Socket is already connected.  */
# define ENOTCONN	134	/* Socket is not connected.  */
/* XENIX has 135 - 142.  */
# define ESHUTDOWN	143	/* Can't send after socket shutdown.  */
# define ETOOMANYREFS	144	/* Too many references: can't splice.  */
# define ETIMEDOUT	145	/* Connection timed out.  */
# define ECONNREFUSED	146	/* Connection refused.  */
# define EHOSTDOWN	147	/* Host is down.  */
# define EHOSTUNREACH	148	/* No route to host.  */
# define EALREADY	149	/* operation already in progress.  */
# define EINPROGRESS	150	/* operation now in progress.  */

/* SUN Network File System.  */
# define ESTALE		151     /* Stale NFS file handle.  */

#endif

#define __set_errno(val) errno = (val)
