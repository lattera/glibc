/* Copyright (C) 1991,92,93,94,95,96,97,98 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 *	POSIX Standard: 2.10 Symbolic Constants		<unistd.h>
 */

#ifndef	_UNISTD_H
#define	_UNISTD_H	1

#include <features.h>

__BEGIN_DECLS

/* These may be used to determine what facilities are present at compile time.
   Their values can be obtained at run time from `sysconf'.  */

/* POSIX Standard approved as ISO/IEC 9945-1 as of August, 1988 and
   extended by POSIX-1b (aka POSIX-4) and POSIX-1c (aka POSIX threads).  */
#define	_POSIX_VERSION	199506L

/* These are not #ifdef __USE_POSIX2 because they are
   in the theoretically application-owned namespace.  */

/* POSIX Standard approved as ISO/IEC 9945-2 as of December, 1993.  */
#define	_POSIX2_C_VERSION	199209L

/* The utilities on GNU systems also correspond to this version.  */
#define _POSIX2_VERSION	199209L

/* If defined, the implementation supports the
   C Language Bindings Option.  */
#define	_POSIX2_C_BIND	1

/* If defined, the implementation supports the
   C Language Development Utilities Option.  */
#define	_POSIX2_C_DEV	1

/* If defined, the implementation supports the
   Software Development Utilities Option.  */
#define	_POSIX2_SW_DEV	1

/* If defined, the implementation supports the
   creation of locales with the localedef utility.  */
#define _POSIX2_LOCALEDEF       1

/* X/Open version number to which the library conforms.  It is selectable.  */
#ifdef __USE_UNIX98
# define _XOPEN_VERSION	500
#else
# define _XOPEN_VERSION	4
#endif

/* Commands and utilities from XPG4 are available.  */
#define _XOPEN_XCU_VERSION	4

/* We are compatible with the old published standards as well.  */
#define _XOPEN_XPG2	1
#define _XOPEN_XPG3	1
#define _XOPEN_XPG4	1

/* The X/Open Unix extensions are available.  */
#define _XOPEN_UNIX	1

/* Encryption is present.  */
#define	_XOPEN_CRYPT	1

/* The enhanced internationalization capabilities according to XPG4.2
   are present.  */
#define	_XOPEN_ENH_I18N	1

/* The legacy interfaces are also available.  */
#define _XOPEN_LEGACY	1


/* Get values of POSIX options:

   If these symbols are defined, the corresponding features are
   always available.  If not, they may be available sometimes.
   The current values can be obtained with `sysconf'.

   _POSIX_JOB_CONTROL		Job control is supported.
   _POSIX_SAVED_IDS		Processes have a saved set-user-ID
   				and a saved set-group-ID.
   _POSIX_REALTIME_SIGNALS	Real-time, queued signals are supported.
   _POSIX_PRIORITY_SCHEDULING	Priority scheduling is supported.
   _POSIX_TIMERS		POSIX.4 clocks and timers are supported.
   _POSIX_ASYNCHRONOUS_IO	Asynchronous I/O is supported.
   _POSIX_PRIORITIZED_IO	Prioritized asynchronous I/O is supported.
   _POSIX_SYNCHRONIZED_IO	Synchronizing file data is supported.
   _POSIX_FSYNC			The fsync function is present.
   _POSIX_MAPPED_FILES		Mapping of files to memory is supported.
   _POSIX_MEMLOCK		Locking of all memory is supported.
   _POSIX_MEMLOCK_RANGE		Locking of ranges of memory is supported.
   _POSIX_MEMORY_PROTECTION	Setting of memory protections is supported.
   _POSIX_MESSAGE_PASSING	POSIX.4 message queues are supported.
   _POSIX_SEMAPHORES		POSIX.4 counting semaphores are supported.
   _POSIX_SHARED_MEMORY_OBJECTS	POSIX.4 shared memory objects are supported.
   _POSIX_THREADS		POSIX.1c pthreads are supported.
   _POSIX_THREAD_ATTR_STACKADDR	Thread stack address attribute option supported.
   _POSIX_THREAD_ATTR_STACKSIZE	Thread stack size attribute option supported.
   _POSIX_THREAD_SAFE_FUNCTIONS	Thread-safe functions are supported.
   _POSIX_THREAD_PRIORITY_SCHEDULING
				POSIX.1c thread execution scheduling supported.
   _POSIX_THREAD_PRIO_INHERIT	Thread priority inheritance option supported.
   _POSIX_THREAD_PRIO_PROTECT	Thread priority protection option supported.
   _POSIX_THREAD_PROCESS_SHARED	Process-shared synchronization supported.
   _POSIX_PII			Protocol-independent interfaces are supported.
   _POSIX_PII_XTI		XTI protocol-indep. interfaces are supported.
   _POSIX_PII_SOCKET		Socket protocol-indep. interfaces are supported.
   _POSIX_PII_INTERNET		Internet family of protocols supported.
   _POSIX_PII_INTERNET_STREAM	Connection-mode Internet protocol supported.
   _POSIX_PII_INTERNET_DGRAM	Connectionless Internet protocol supported.
   _POSIX_PII_OSI		ISO/OSI family of protocols supported.
   _POSIX_PII_OSI_COTS		Connection-mode ISO/OSI service supported.
   _POSIX_PII_OSI_CLTS		Connectionless ISO/OSI service supported.
   _POSIX_POLL			Implementation supports `poll' function.
   _POSIX_SELECT		Implementation supports `select' and `pselect'.

   _XOPEN_REALTIME		X/Open realtime support is available.
   _XOPEN_REALTIME_THREADS	X/Open realtime thread support is available.
   _XOPEN_SHM			Shared memory interface according to XPG4.2.

   _XBS5_ILP32_OFF32		Implementation provides environment with 32-bit
				int, long, pointer, and off_t types.
   _XBS5_ILP32_OFFBIG		Implementation provides environment with 32-bit
				int, long, and pointer and off_t with at least
				64 bits.
   _XBS5_LP64_OFF64		Implementation provides environment with 32-bit
				int, and 64-bit long, pointer, and off_t types.
   _XBS5_LPBIG_OFFBIG		Implementation provides environment with at
				least 32 bits int and long, pointer, and off_t
				with at least 64 bits.

   If any of these symbols is defined as -1, the corresponding option is not
   true for any file.  If any is defined as other than -1, the corresponding
   option is true for all files.  If a symbol is not defined at all, the value
   for a specific file can be obtained from `pathconf' and `fpathconf'.

   _POSIX_CHOWN_RESTRICTED	Only the super user can use `chown' to change
   				the owner of a file.  `chown' can only be used
				to change the group ID of a file to a group of
				which the calling process is a member.
   _POSIX_NO_TRUNC		Pathname components longer than
   				NAME_MAX generate an error.
   _POSIX_VDISABLE		If defined, if the value of an element of the
				`c_cc' member of `struct termios' is
				_POSIX_VDISABLE, no character will have the
				effect associated with that element.
   _POSIX_SYNC_IO		Synchronous I/O may be performed.
   _POSIX_ASYNC_IO		Asynchronous I/O may be performed.
   _POSIX_PRIO_IO		Prioritized Asynchronous I/O may be performed.

   Support for the Large File Support interface is not generally available.
   If it is available the following constants are defined to one.
   _LFS64_LARGEFILE		Low-level I/O supports large files.
   _LFS64_STDIO			Standard I/O supports large files.
   */

#include <bits/posix_opt.h>

/* Get the environment definitions from Unix98.  */
#ifdef __USE_UNIX98
# include <bits/environments.h>
#endif

/* Standard file descriptors.  */
#define	STDIN_FILENO	0	/* Standard input.  */
#define	STDOUT_FILENO	1	/* Standard output.  */
#define	STDERR_FILENO	2	/* Standard error output.  */


/* All functions that are not declared anywhere else.  */

#include <bits/types.h>

#ifndef	ssize_t
typedef __ssize_t ssize_t;
# define ssize_t ssize_t
#endif

#define	__need_size_t
#define __need_NULL
#include <stddef.h>

#ifdef __USE_XOPEN
/* The Single Unix specification says that some more types are
   available here.  */
# ifndef gid_t
typedef __gid_t gid_t;
#  define gid_t gid_t
# endif

# ifndef uid_t
typedef __uid_t uid_t;
#  define uid_t uid_t
# endif

# ifndef off_t
#  ifndef __USE_FILE_OFFSET64
typedef __off_t off_t;
#  else
typedef __off64_t off_t;
#  endif
#  define off_t off_t
# endif
# if defined __USE_LARGEFILE64 && !defined off64_t
typedef __off64_t off64_t;
#  define off64_t off64_t
# endif

# ifndef pid_t
typedef __pid_t pid_t;
#  define pid_t pid_t
# endif
#endif	/* X/Open */

#ifdef __USE_UNIX98
# ifndef intptr_t
typedef __intptr_t intptr_t;
#  define intptr_t intptr_t
# endif
#endif	/* Unix98 */

/* Values for the second argument to access.
   These may be OR'd together.  */
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */

/* Test for access to NAME using the real UID and real GID.  */
extern int access __P ((__const char *__name, int __type));

#ifdef __USE_GNU
/* Test for access to NAME using the effective UID and GID
   (as normal file operations use).  */
extern int euidaccess __P ((__const char *__name, int __type));
#endif


/* Values for the WHENCE argument to lseek.  */
#ifndef	_STDIO_H		/* <stdio.h> has the same definitions.  */
# define SEEK_SET	0	/* Seek from beginning of file.  */
# define SEEK_CUR	1	/* Seek from current position.  */
# define SEEK_END	2	/* Seek from end of file.  */
#endif

#if defined __USE_BSD && !defined L_SET
/* Old BSD names for the same constants; just for compatibility.  */
# define L_SET		SEEK_SET
# define L_INCR		SEEK_CUR
# define L_XTND		SEEK_END
#endif


/* Move FD's file position to OFFSET bytes from the
   beginning of the file (if WHENCE is SEEK_SET),
   the current position (if WHENCE is SEEK_CUR),
   or the end of the file (if WHENCE is SEEK_END).
   Return the new file position.  */
extern __off_t __lseek __P ((int __fd, __off_t __offset, int __whence));
#ifndef __USE_FILE_OFFSET64
extern __off_t lseek __P ((int __fd, __off_t __offset, int __whence));
#else
# ifdef __REDIRECT
extern __off64_t __REDIRECT (lseek,
			     __P ((int __fd, __off64_t __offset,
				   int __whence)),
			     lseek64);
# else
#  define lseek lseek64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern __off64_t lseek64 __P ((int __fd, __off64_t __offset, int __whence));
#endif

/* Close the file descriptor FD.  */
extern int __close __P ((int __fd));
extern int close __P ((int __fd));

/* Read NBYTES into BUF from FD.  Return the
   number read, -1 for errors or 0 for EOF.  */
extern ssize_t __read __P ((int __fd, __ptr_t __buf, size_t __nbytes));
extern ssize_t read __P ((int __fd, __ptr_t __buf, size_t __nbytes));

/* Write N bytes of BUF to FD.  Return the number written, or -1.  */
extern ssize_t __write __P ((int __fd, __const __ptr_t __buf, size_t __n));
extern ssize_t write __P ((int __fd, __const __ptr_t __buf, size_t __n));

#ifdef __USE_UNIX98
/* Read NBYTES into BUF from FD at the given position OFFSET without
   changing the file pointer.  Return the number read, -1 for errors
   or 0 for EOF.  */
extern ssize_t __pread64 __P ((int __fd, __ptr_t __buf, size_t __nbytes,
			       __off64_t __offset));
/* Write N bytes of BUF to FD at the given position OFFSET without
   changing the file pointer.  Return the number written, or -1.  */
extern ssize_t __pwrite64 __P ((int __fd, __const __ptr_t __buf, size_t __n,
				__off64_t __offset));

# ifndef __USE_FILE_OFFSET64
extern ssize_t pread __P ((int __fd, __ptr_t __buf, size_t __nbytes,
			   __off_t __offset));
extern ssize_t pwrite __P ((int __fd, __const __ptr_t __buf, size_t __n,
			    __off_t __offset));
# else
#  ifdef __REDIRECT
extern ssize_t __REDIRECT (pread, __P ((int __fd, __ptr_t __buf,
					size_t __nbytes,__off64_t __offset)),
			   pread64);
extern ssize_t __REDIRECT (pwrite, __P ((int __fd, __const __ptr_t __buf,
					 size_t __nbytes, __off64_t __offset)),
			pwrite64);
#  else
#   define pread pread64
#   define pwrite pwrite64
#  endif
# endif

# ifdef __USE_LARGEFILE64
extern ssize_t pread64 __P ((int __fd, __ptr_t __buf, size_t __nbytes,
			     __off64_t __offset));
extern ssize_t pwrite64 __P ((int __fd, __const __ptr_t __buf, size_t __n,
			      __off64_t __offset));
# endif
#endif

/* Create a one-way communication channel (pipe).
   If successful, two file descriptors are stored in PIPEDES;
   bytes written on PIPEDES[1] can be read from PIPEDES[0].
   Returns 0 if successful, -1 if not.  */
extern int pipe __P ((int __pipedes[2]));

/* Schedule an alarm.  In SECONDS seconds, the process will get a SIGALRM.
   If SECONDS is zero, any currently scheduled alarm will be cancelled.
   The function returns the number of seconds remaining until the last
   alarm scheduled would have signaled, or zero if there wasn't one.
   There is no return value to indicate an error, but you can set `errno'
   to 0 and check its value after calling `alarm', and this might tell you.
   The signal may come late due to processor scheduling.  */
extern unsigned int alarm __P ((unsigned int __seconds));

/* Make the process sleep for SECONDS seconds, or until a signal arrives
   and is not ignored.  The function returns the number of seconds less
   than SECONDS which it actually slept (thus zero if it slept the full time).
   If a signal handler does a `longjmp' or modifies the handling of the
   SIGALRM signal while inside `sleep' call, the handling of the SIGALRM
   signal afterwards is undefined.  There is no return value to indicate
   error, but if `sleep' returns SECONDS, it probably didn't work.  */
extern unsigned int sleep __P ((unsigned int __seconds));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Set an alarm to go off (generating a SIGALRM signal) in VALUE
   microseconds.  If INTERVAL is nonzero, when the alarm goes off, the
   timer is reset to go off every INTERVAL microseconds thereafter.
   Returns the number of microseconds remaining before the alarm.  */
extern unsigned int ualarm __P ((unsigned int __value,
				 unsigned int __interval));

/* Sleep USECONDS microseconds, or until a signal arrives that is not blocked
   or ignored.  */
extern void usleep __P ((unsigned int __useconds));
#endif


/* Suspend the process until a signal arrives.
   This always returns -1 and sets `errno' to EINTR.  */
extern int pause __P ((void));


/* Change the owner and group of FILE.  */
extern int chown __P ((__const char *__file, __uid_t __owner,
		       __gid_t __group));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Change the owner and group of the file that FD is open on.  */
extern int fchown __P ((int __fd, __uid_t __owner, __gid_t __group));


/* Change owner and group of FILE, if it is a symbolic
   link the ownership of the symbolic link is changed.  */
extern int lchown __P ((__const char *__file, __uid_t __owner,
			__gid_t __group));

#endif /* Use BSD || X/Open Unix.  */

/* Change the process's working directory to PATH.  */
extern int chdir __P ((__const char *__path));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Change the process's working directory to the one FD is open on.  */
extern int fchdir __P ((int __fd));
#endif

/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE == 0, in which case it is as
   big as necessary.  */
extern char *getcwd __P ((char *__buf, size_t __size));

#ifdef	__USE_GNU
/* Return a malloc'd string containing the current directory name.
   If the environment variable `PWD' is set, and its value is correct,
   that value is used.  */
extern char *get_current_dir_name __P ((void));
#endif

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Put the absolute pathname of the current working directory in BUF.
   If successful, return BUF.  If not, put an error message in
   BUF and return NULL.  BUF should be at least PATH_MAX bytes long.  */
extern char *getwd __P ((char *__buf));
#endif


/* Duplicate FD, returning a new file descriptor on the same file.  */
extern int dup __P ((int __fd));

/* Duplicate FD to FD2, closing FD2 and making it open on the same file.  */
extern int dup2 __P ((int __fd, int __fd2));

/* NULL-terminated array of "NAME=VALUE" environment variables.  */
extern char **__environ;
#ifdef __USE_GNU
extern char **environ;
#endif


/* Replace the current process, executing PATH with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
extern int execve __P ((__const char *__path, char *__const __argv[],
			char *__const __envp[]));

#ifdef __USE_GNU
/* Execute the file FD refers to, overlaying the running program image.
   ARGV and ENVP are passed to the new program, as for `execve'.  */
extern int fexecve __P ((int __fd,
			 char *__const __argv[], char *__const __envp[]));

#endif


/* Execute PATH with arguments ARGV and environment from `environ'.  */
extern int execv __P ((__const char *__path, char *__const __argv[]));

/* Execute PATH with all arguments after PATH until a NULL pointer,
   and the argument after that for environment.  */
extern int execle __P ((__const char *__path, __const char *__arg, ...));

/* Execute PATH with all arguments after PATH until
   a NULL pointer and environment from `environ'.  */
extern int execl __P ((__const char *__path, __const char *__arg, ...));

/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from `environ'.  */
extern int execvp __P ((__const char *__file, char *__const __argv[]));

/* Execute FILE, searching in the `PATH' environment variable if
   it contains no slashes, with all arguments after FILE until a
   NULL pointer and environment from `environ'.  */
extern int execlp __P ((__const char *__file, __const char *__arg, ...));


#if defined __USE_MISC || defined __USE_XOPEN
/* Add INC to priority of the current process.  */
extern int nice __P ((int __inc));
#endif


/* Terminate program execution with the low-order 8 bits of STATUS.  */
extern void _exit __P ((int __status)) __attribute__ ((__noreturn__));


/* Get the `_PC_*' symbols for the NAME argument to `pathconf' and `fpathconf';
   the `_SC_*' symbols for the NAME argument to `sysconf';
   and the `_CS_*' symbols for the NAME argument to `confstr'.  */
#include <bits/confname.h>

/* Get file-specific configuration information about PATH.  */
extern long int pathconf __P ((__const char *__path, int __name));

/* Get file-specific configuration about descriptor FD.  */
extern long int fpathconf __P ((int __fd, int __name));

/* Get the value of the system variable NAME.  */
extern long int sysconf __P ((int __name));

#ifdef	__USE_POSIX2
/* Get the value of the string-valued system variable NAME.  */
extern size_t confstr __P ((int __name, char *__buf, size_t __len));
#endif


/* Get the process ID of the calling process.  */
extern __pid_t __getpid __P ((void));
extern __pid_t getpid __P ((void));

/* Get the process ID of the calling process's parent.  */
extern __pid_t getppid __P ((void));

/* Get the process group ID of the calling process.
   This function is different on old BSD. */
#ifndef __FAVOR_BSD
extern __pid_t getpgrp __P ((void));
#else
# ifdef __REDIRECT
extern __pid_t __REDIRECT (getpgrp, __P ((__pid_t __pid)), __getpgid);
# else
#  define getpgrp __getpgid
# endif
#endif

/* Get the process group ID of process PID.  */
extern __pid_t __getpgid __P ((__pid_t __pid));
#ifdef __USE_XOPEN_EXTENDED
extern __pid_t getpgid __P ((__pid_t __pid));
#endif


/* Set the process group ID of the process matching PID to PGID.
   If PID is zero, the current process's process group ID is set.
   If PGID is zero, the process ID of the process is used.  */
extern int __setpgid __P ((__pid_t __pid, __pid_t __pgid));
extern int setpgid __P ((__pid_t __pid, __pid_t __pgid));

#if defined __USE_SVID || defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Both System V and BSD have `setpgrp' functions, but with different
   calling conventions.  The BSD function is the same as POSIX.1 `setpgid'
   (above).  The System V function takes no arguments and puts the calling
   process in its on group like `setpgid (0, 0)'.

   New programs should always use `setpgid' instead.

   The default in GNU is to provide the System V function.  The BSD
   function is available under -D_BSD_SOURCE.  */

# ifndef __FAVOR_BSD

/* Set the process group ID of the calling process to its own PID.
   This is exactly the same as `setpgid (0, 0)'.  */
extern int setpgrp __P ((void));

# else

/* Another name for `setpgid' (above).  */
#  ifdef __REDIRECT
extern int __REDIRECT (setpgrp, __P ((__pid_t __pid, __pid_t __pgrp)),
		       setpgid);
#  else
#   define setpgrp setpgid
#  endif

# endif	/* Favor BSD.  */
#endif	/* Use SVID or BSD.  */

/* Create a new session with the calling process as its leader.
   The process group IDs of the session and the calling process
   are set to the process ID of the calling process, which is returned.  */
extern __pid_t setsid __P ((void));

#ifdef __USE_XOPEN_EXTENDED
/* Return the session ID of the given process.  */
extern __pid_t getsid __P ((__pid_t __pid));
#endif

/* Get the real user ID of the calling process.  */
extern __uid_t getuid __P ((void));

/* Get the effective user ID of the calling process.  */
extern __uid_t geteuid __P ((void));

/* Get the real group ID of the calling process.  */
extern __gid_t getgid __P ((void));

/* Get the effective group ID of the calling process.  */
extern __gid_t getegid __P ((void));

/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
extern int getgroups __P ((int __size, __gid_t __list[]));

#ifdef	__USE_GNU
/* Return nonzero iff the calling process is in group GID.  */
extern int group_member __P ((__gid_t __gid));
#endif

/* Set the user ID of the calling process to UID.
   If the calling process is the super-user, set the real
   and effective user IDs, and the saved set-user-ID to UID;
   if not, the effective user ID is set to UID.  */
extern int setuid __P ((__uid_t __uid));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Set the real user ID of the calling process to RUID,
   and the effective user ID of the calling process to EUID.  */
extern int setreuid __P ((__uid_t __ruid, __uid_t __euid));
#endif

#ifdef	__USE_BSD
/* Set the effective user ID of the calling process to UID.  */
extern int seteuid __P ((__uid_t __uid));
#endif /* Use BSD.  */

/* Set the group ID of the calling process to GID.
   If the calling process is the super-user, set the real
   and effective group IDs, and the saved set-group-ID to GID;
   if not, the effective group ID is set to GID.  */
extern int setgid __P ((__gid_t __gid));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Set the real group ID of the calling process to RGID,
   and the effective group ID of the calling process to EGID.  */
extern int setregid __P ((__gid_t __rgid, __gid_t __egid));
#endif

#ifdef __USE_BSD
/* Set the effective group ID of the calling process to GID.  */
extern int setegid __P ((__gid_t __gid));
#endif /* Use BSD.  */


/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
extern __pid_t __fork __P ((void));
extern __pid_t fork __P ((void));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
extern __pid_t vfork __P ((void));
#endif /* Use BSD. */


/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
extern char *ttyname __P ((int __fd));

/* Store at most BUFLEN characters of the pathname of the terminal FD is
   open on in BUF.  Return 0 on success, otherwise an error number.  */
extern int ttyname_r __P ((int __fd, char *__buf, size_t __buflen));

/* Return 1 if FD is a valid descriptor associated
   with a terminal, zero if not.  */
extern int isatty __P ((int __fd));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Return the index into the active-logins file (utmp) for
   the controlling terminal.  */
extern int ttyslot __P ((void));
#endif


/* Make a link to FROM named TO.  */
extern int link __P ((__const char *__from, __const char *__to));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Make a symbolic link to FROM named TO.  */
extern int symlink __P ((__const char *__from, __const char *__to));

/* Read the contents of the symbolic link PATH into no more than
   LEN bytes of BUF.  The contents are not null-terminated.
   Returns the number of characters read, or -1 for errors.  */
extern int readlink __P ((__const char *__path, char *__buf, size_t __len));
#endif /* Use BSD.  */

/* Remove the link NAME.  */
extern int unlink __P ((__const char *__name));

/* Remove the directory PATH.  */
extern int rmdir __P ((__const char *__path));


/* Return the foreground process group ID of FD.  */
extern __pid_t tcgetpgrp __P ((int __fd));

/* Set the foreground process group ID of FD set PGRP_ID.  */
extern int tcsetpgrp __P ((int __fd, __pid_t __pgrp_id));


/* Return the login name of the user.  */
extern char *getlogin __P ((void));
#ifdef __USE_REENTRANT
/* Return at most NAME_LEN characters of the login name of the user in NAME.
   If it cannot be determined or some other error occurred, return the error
   code.  Otherwise return 0.  */
extern int getlogin_r __P ((char *__name, size_t __name_len));
#endif

#ifdef	__USE_BSD
/* Set the login name returned by `getlogin'.  */
extern int setlogin __P ((__const char *__name));
#endif


#ifdef	__USE_POSIX2
/* Get definitions and prototypes for functions to process the
   arguments in ARGV (ARGC of them, minus the program name) for
   options given in OPTS.  */
# define __need_getopt
# include <getopt.h>
#endif


#if defined __USE_BSD || defined __USE_XOPEN

/* Put the name of the current host in no more than LEN bytes of NAME.
   The result is null-terminated if LEN is large enough for the full
   name and the terminator.  */
extern int gethostname __P ((char *__name, size_t __len));

/* Set the name of the current host to NAME, which is LEN bytes long.
   This call is restricted to the super-user.  */
extern int sethostname __P ((__const char *__name, size_t __len));

/* Set the current machine's Internet number to ID.
   This call is restricted to the super-user.  */
extern int sethostid __P ((long int __id));


/* Get and set the NIS (aka YP) domain name, if any.
   Called just like `gethostname' and `sethostname'.
   The NIS domain name is usually the empty string when not using NIS.  */
extern int getdomainname __P ((char *__name, size_t __len));
extern int setdomainname __P ((__const char *__name, size_t __len));


/* Make all changes done to FD actually appear on disk.  */
extern int fsync __P ((int __fd));


/* Revoke access permissions to all processes currently communicating
   with the control terminal, and then send a SIGHUP signal to the process
   group of the control terminal.  */
extern int vhangup __P ((void));

/* Revoke the access of all descriptors currently open on FILE.  */
extern int revoke __P ((__const char *__file));


/* Enable statistical profiling, writing samples of the PC into at most
   SIZE bytes of SAMPLE_BUFFER; every processor clock tick while profiling
   is enabled, the system examines the user PC and increments
   SAMPLE_BUFFER[((PC - OFFSET) / 2) * SCALE / 65536].  If SCALE is zero,
   disable profiling.  Returns zero on success, -1 on error.  */
extern int profil __P ((unsigned short int *__sample_buffer, size_t __size,
			size_t __offset, unsigned int __scale));


/* Turn accounting on if NAME is an existing file.  The system will then write
   a record for each process as it terminates, to this file.  If NAME is NULL,
   turn accounting off.  This call is restricted to the super-user.  */
extern int acct __P ((__const char *__name));

/* Make PATH be the root directory (the starting point for absolute paths).
   This call is restricted to the super-user.  */
extern int chroot __P ((__const char *__path));


/* Successive calls return the shells listed in `/etc/shells'.  */
extern char *getusershell __P ((void));
extern void endusershell __P ((void)); /* Discard cached info.  */
extern void setusershell __P ((void)); /* Rewind and re-read the file.  */


/* Prompt with PROMPT and read a string from the terminal without echoing.
   Uses /dev/tty if possible; otherwise stderr and stdin.  */
extern char *getpass __P ((__const char *__prompt));

/* Put the program in the background, and dissociate from the controlling
   terminal.  If NOCHDIR is zero, do `chdir ("/")'.  If NOCLOSE is zero,
   redirects stdin, stdout, and stderr to /dev/null.  */
extern int daemon __P ((int __nochdir, int __noclose));

#endif /* Use BSD || X/Open.  */


#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED

/* Return the current machine's Internet number.  */
extern long int gethostid __P ((void));

/* Make all changes done to all files actually appear on disk.  */
extern int sync __P ((void));


/* Return the number of bytes in a page.  This is the system's page size,
   which is not necessarily the same as the hardware page size.  */
extern int __getpagesize __P ((void));
extern int getpagesize __P ((void));


/* Truncate FILE to LENGTH bytes.  */
#ifndef __USE_FILE_OFFSET64
extern int truncate __P ((__const char *__file, __off_t __length));
#else
# ifdef __REDIRECT
extern int __REDIRECT (truncate,
		       __P ((__const char *__file, __off64_t __length)),
		       truncate64);
# else
#  define truncate truncate64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int truncate64 __P ((__const char *__file, __off64_t __length));
#endif

/* Truncate the file FD is open on to LENGTH bytes.  */
extern int __ftruncate __P ((int __fd, __off_t __length));
#ifndef __USE_FILE_OFFSET64
extern int ftruncate __P ((int __fd, __off_t __length));
#else
# ifdef __REDIRECT
extern int __REDIRECT (ftruncate, __P ((int __fd, __off64_t __length)),
		       ftruncate64);
# else
#  define ftruncate ftruncate64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int ftruncate64 __P ((int __fd, __off64_t __length));
#endif


/* Return the maximum number of file descriptors
   the current process could possibly have.  */
extern int getdtablesize __P ((void));

#endif /* Use BSD || X/Open Unix.  */


#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED

/* Set the end of accessible data space (aka "the break") to ADDR.
   Returns zero on success and -1 for errors (with errno set).  */
extern int brk __P ((__ptr_t __addr));

# define __need_ptrdiff_t
# include <stddef.h>

/* Increase or decrease the end of accessible data space by DELTA bytes.
   If successful, returns the address the previous end of data space
   (i.e. the beginning of the new space, if DELTA > 0);
   returns (void *) -1 for errors (with errno set).  */
extern __ptr_t __sbrk __P ((ptrdiff_t __delta));
extern __ptr_t sbrk __P ((ptrdiff_t __delta));
#endif


#ifdef __USE_MISC
/* Invoke `system call' number SYSNO, passing it the remaining arguments.
   This is completely system-dependent, and not often useful.

   In Unix, `syscall' sets `errno' for all errors and most calls return -1
   for errors; in many systems you cannot pass arguments or get return
   values for all system calls (`pipe', `fork', and `getppid' typically
   among them).

   In Mach, all system calls take normal arguments and always return an
   error code (zero for success).  */
extern long int syscall __P ((long int __sysno, ...));

#endif	/* Use misc.  */


#if (defined __USE_MISC || defined __USE_XOPEN_EXTENDED) && !defined F_LOCK
/* NOTE: These declarations also appear in <fcntl.h>; be sure to keep both
   files consistent.  Some systems have them there and some here, and some
   software depends on the macros being defined without including both.  */

/* `lockf' is a simpler interface to the locking facilities of `fcntl'.
   LEN is always relative to the current file position.
   The CMD argument is one of the following.  */

# define F_ULOCK 0	/* Unlock a previously locked region.  */
# define F_LOCK  1	/* Lock a region for exclusive use.  */
# define F_TLOCK 2	/* Test and lock a region for exclusive use.  */
# define F_TEST  3	/* Test a region for other processes locks.  */

# ifndef __USE_FILE_OFFSET64
extern int lockf __P ((int __fd, int __cmd, __off_t __len));
# else
#  ifdef __REDIRECT
extern int __REDIRECT (lockf, __P ((int __fd, int __cmd, __off64_t __len)),
		       lockf64);
#  else
#   define lockf lockf64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int lockf64 __P ((int __fd, int __cmd, __off64_t __len));
# endif
#endif /* Use misc and F_LOCK not already defined.  */


#ifdef __USE_GNU

/* Evaluate EXPRESSION, and repeat as long as it returns -1 with `errno'
   set to EINTR.  */

# define TEMP_FAILURE_RETRY(expression) \
  (__extension__							      \
    ({ long int __result;						      \
       do __result = (long int) (expression);				      \
       while (__result == -1L && errno == EINTR);			      \
       __result; }))							      \


/* This variable is set nonzero at startup if the process's effective
   IDs differ from its real IDs, or it is otherwise indicated that
   extra security should be used.  When this is set the dynamic linker
   and some functions contained in the C library ignore various
   environment variables that normally affect them.  */
extern int __libc_enable_secure;

#endif

#if defined __USE_POSIX199309 || defined __USE_UNIX98
/* Synchronize at least the data part of a file with the underlying
   media.  */
extern int fdatasync __P ((int __fildes));
#endif /* Use POSIX199309 */


/* XPG4.2 specifies that prototypes for the encryption functions must
   be defined here.  */
#ifdef	__USE_XOPEN
/* Encrypt at most 8 characters from KEY using salt to perturb DES.  */
extern char *crypt __P ((__const char *__key, __const char *__salt));

/* Setup DES tables according KEY.  */
extern void setkey __P ((__const char *__key));

/* Encrypt data in BLOCK in place if EDFLAG is zero; otherwise decrypt
   block in place.  */
extern void encrypt __P ((char *__block, int __edflag));


/* Swab pairs bytes in the first N bytes of the area pointed to by
   FROM and copy the result to TO.  The value of TO must not be in the
   range [FROM - N + 1, FROM - 1].  If N is odd the first byte in FROM
   is without partner.  */
extern void swab __P ((__const __ptr_t __from, __ptr_t __to, ssize_t __n));
#endif


/* The Single Unix specification, version 2, demands these prototypes
   to be here.  They are also found in <stdio.h>.  */
#ifdef __USE_XOPEN
/* Return the name of the controlling terminal.  */
extern char *ctermid __P ((char *__s));

/* Return the name of the current user.  This function should not be
   used and might go away some time.  */
extern char *cuserid __P ((char *__s));

/* Rename file OLD to NEW.  */
extern int rename __P ((__const char *__old, __const char *__new));
#endif


#ifdef __USE_POSIX199309
/* This function is only available if the system has POSIX threads.  */

/* Install handlers to be called when a new process is created with FORK.
   The PREPARE handler is called in the parent process just before performing
   FORK. The PARENT handler is called in the parent process just after FORK.
   The CHILD handler is called in the child process.  Each of the three
   handlers can be NULL, meaning that no handler needs to be called at that
   point.
   PTHREAD_ATFORK can be called several times, in which case the PREPARE
   handlers are called in LIFO order (last added with PTHREAD_ATFORK,
   first called before FORK), and the PARENT and CHILD handlers are called
   in FIFO (first added, first called). */
extern int pthread_atfork __P ((void (*__prepare) (void),
				void (*__parent) (void),
				void (*__child) (void)));
#endif

__END_DECLS

#endif /* unistd.h  */
