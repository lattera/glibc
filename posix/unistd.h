/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	POSIX Standard: 2.10 Symbolic Constants		<unistd.h>
 */

#ifndef	_UNISTD_H

#define	_UNISTD_H	1
#include <features.h>

__BEGIN_DECLS

/* These may be used to determine what facilities are present at compile time.
   Their values can be obtained at run time from sysconf.  */

/* POSIX Standard approved as IEEE Std 1003.1 as of August, 1988.  */
#define	_POSIX_VERSION	199009L

/* These are not #ifdef __USE_POSIX2 because they are
   in the theoretically application-owned namespace.  */

#define	_POSIX2_C_VERSION	199912L	/* Invalid until 1003.2 is done.  */

/* If defined, the implementation supports the
   C Language Bindings Option.  */
#define	_POSIX2_C_BIND	1

/* If defined, the implementation supports the
   C Language Development Utilities Option.  */
#define	_POSIX2_C_DEV	1

/* If defined, the implementation supports the
   Software Development Utilities Option.  */
#define	_POSIX2_SW_DEV	1


/* Get values of POSIX options:

   If these symbols are defined, the corresponding features are
   always available.  If not, they may be available sometimes.
   The current values can be obtained with `sysconf'.

   _POSIX_JOB_CONTROL	Job control is supported.
   _POSIX_SAVED_IDS	Processes have a saved set-user-ID
   			and a saved set-group-ID.

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
   */

#include <posix_opt.h>


/* Standard file descriptors.  */
#define	STDIN_FILENO	0	/* Standard input.  */
#define	STDOUT_FILENO	1	/* Standard output.  */
#define	STDERR_FILENO	2	/* Standard error output.  */


/* All functions that are not declared anywhere else.  */

#include <gnu/types.h>

#ifndef	ssize_t
#define	ssize_t	__ssize_t
#endif

#define	__need_size_t
#define __need_NULL
#include <stddef.h>


/* Values for the second argument to access.
   These may be OR'd together.  */
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */

/* Test for access to NAME using the real UID and real GID.  */
extern int __access __P ((__const char *__name, int __type));
extern int access __P ((__const char *__name, int __type));

#ifdef __USE_GNU
/* Test for access to NAME using the effective UID and GID
   (as normal file operations use).  */
extern int euidaccess __P ((__const char *__name, int __type));
#endif


/* Values for the WHENCE argument to lseek.  */
#ifndef	_STDIO_H		/* <stdio.h> has the same definitions.  */
#define	SEEK_SET	0	/* Seek from beginning of file.  */
#define	SEEK_CUR	1	/* Seek from current position.  */
#define	SEEK_END	2	/* Seek from end of file.  */
#endif

/* Move FD's file position to OFFSET bytes from the
   beginning of the file (if WHENCE is SEEK_SET),
   the current position (if WHENCE is SEEK_CUR),
   or the end of the file (if WHENCE is SEEK_END).
   Return the new file position.  */
extern __off_t __lseek __P ((int __fd, __off_t __offset, int __whence));
extern __off_t lseek __P ((int __fd, __off_t __offset, int __whence));

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


/* Create a one-way communication channel (pipe).
   If successul, two file descriptors are stored in PIPEDES;
   bytes written on PIPEDES[1] can be read from PIPEDES[0].
   Returns 0 if successful, -1 if not.  */
extern int __pipe __P ((int __pipedes[2]));
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


/* Suspend the process until a signal arrives.
   This always returns -1 and sets `errno' to EINTR.  */
extern int pause __P ((void));


/* Change the owner and group of FILE.  */
extern int __chown __P ((__const char *__file,
			 __uid_t __owner, __gid_t __group));
extern int chown __P ((__const char *__file,
		       __uid_t __owner, __gid_t __group));

#ifdef	__USE_BSD
/* Change the owner and group of the file that FD is open on.  */
extern int __fchown __P ((int __fd,
			  __uid_t __owner, __gid_t __group));
extern int fchown __P ((int __fd,
			__uid_t __owner, __gid_t __group));
#endif /* Use BSD.  */

/* Change the process's working directory to PATH.  */
extern int __chdir __P ((__const char *__path));
extern int chdir __P ((__const char *__path));

#ifdef __USE_BSD
/* Change the process's working directory to the one FD is open on.  */
extern int fchdir __P ((int __fd));
#endif

/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE <= 0, in which case it is as
   big as necessary.  */
extern char *getcwd __P ((char *__buf, size_t __size));

#ifdef	__USE_GNU
/* Return a malloc'd string containing the current directory name.
   If the environment variable `PWD' is set, and its value is correct,
   that value is used.  */
extern char *get_current_dir_name __P ((void));
#endif

#ifdef	__USE_BSD
/* Put the absolute pathname of the current working directory in BUF.
   If successful, return BUF.  If not, put an error message in
   BUF and return NULL.  BUF should be at least PATH_MAX bytes long.  */
extern char *getwd __P ((char *__buf));
#endif


/* Duplicate FD, returning a new file descriptor on the same file.  */
extern int __dup __P ((int __fd));
extern int dup __P ((int __fd));

/* Duplicate FD to FD2, closing FD2 and making it open on the same file.  */
extern int __dup2 __P ((int __fd, int __fd2));
extern int dup2 __P ((int __fd, int __fd2));

/* NULL-terminated array of "NAME=VALUE" environment variables.  */
extern char **__environ;
extern char **environ;


/* Replace the current process, executing PATH with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
extern int __execve __P ((__const char *__path, char *__const __argv[],
			  char *__const __envp[]));
extern int execve __P ((__const char *__path, char *__const __argv[],
			char *__const __envp[]));

#ifdef __USE_GNU
/* Execute the file FD refers to, overlaying the running program image.
   ARGV and ENVP are passed to the new program, as for `execve'.  */
extern int fexecve __P ((int __fd,
			 char *const __argv[], char *const __envp[]));

#endif


/* Execute PATH with arguments ARGV and environment from `environ'.  */
extern int execv __P ((__const char *__path, char *__const __argv[]));

/* Execute PATH with all arguments after PATH until a NULL pointer,
   and the argument after that for environment.  */
extern int execle __P ((__const char *__path, __const char *__arg,...));

/* Execute PATH with all arguments after PATH until
   a NULL pointer and environment from `environ'.  */
extern int execl __P ((__const char *__path, __const char *__arg,...));

/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from `environ'.  */
extern int execvp __P ((__const char *__file, char *__const __argv[]));

/* Execute FILE, searching in the `PATH' environment variable if
   it contains no slashes, with all arguments after FILE until a
   NULL pointer and environment from `environ'.  */
extern int execlp __P ((__const char *__file, ...));


/* Terminate program execution with the low-order 8 bits of STATUS.  */
extern void _exit __P ((int __status)) __attribute__ ((__noreturn__));


/* Get the `_PC_*' symbols for the NAME argument to `pathconf' and `fpathconf';
   the `_SC_*' symbols for the NAME argument to `sysconf';
   and the `_CS_*' symbols for the NAME argument to `confstr'.  */
#include <confname.h>

/* Get file-specific configuration information about PATH.  */
extern long int __pathconf __P ((__const char *__path, int __name));
extern long int pathconf __P ((__const char *__path, int __name));

/* Get file-specific configuration about descriptor FD.  */
extern long int __fpathconf __P ((int __fd, int __name));
extern long int fpathconf __P ((int __fd, int __name));

/* Get the value of the system variable NAME.  */
extern long int __sysconf __P ((int __name));
extern long int sysconf __P ((int __name));

#ifdef	__USE_POSIX2
/* Get the value of the string-valued system variable NAME.  */
extern size_t confstr __P ((int __name, char *__buf, size_t __len));
#endif


/* Get the process ID of the calling process.  */
extern __pid_t __getpid __P ((void));
extern __pid_t getpid __P ((void));

/* Get the process ID of the calling process's parent.  */
extern __pid_t __getppid __P ((void));
extern __pid_t getppid __P ((void));

/* Get the process group ID of the calling process.  */
extern __pid_t getpgrp __P ((void));

/* Set the process group ID of the process matching PID to PGID.
   If PID is zero, the current process's process group ID is set.
   If PGID is zero, the process ID of the process is used.  */
extern int setpgid __P ((__pid_t __pid, __pid_t __pgid));

/* Get the process group ID of process PID.  */
extern __pid_t __getpgid __P ((__pid_t __pid));
#ifdef __USE_GNU
extern __pid_t getpgid __P ((__pid_t __pid));
#endif

#ifdef	__USE_BSD
/* Another name for `setpgid'.  */
extern int setpgrp __P ((__pid_t __pid, __pid_t __pgrp));
#endif /* Use BSD.  */

/* Create a new session with the calling process as its leader.
   The process group IDs of the session and the calling process
   are set to the process ID of the calling process, which is returned.  */
extern __pid_t __setsid __P ((void));
extern __pid_t setsid __P ((void));

/* Get the real user ID of the calling process.  */
extern __uid_t __getuid __P ((void));
extern __uid_t getuid __P ((void));

/* Get the effective user ID of the calling process.  */
extern __uid_t __geteuid __P ((void));
extern __uid_t geteuid __P ((void));

/* Get the real group ID of the calling process.  */
extern __gid_t __getgid __P ((void));
extern __gid_t getgid __P ((void));

/* Get the effective group ID of the calling process.  */
extern __gid_t __getegid __P ((void));
extern __gid_t getegid __P ((void));

/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
extern int __getgroups __P ((int __size, __gid_t __list[]));
extern int getgroups __P ((int __size, __gid_t __list[]));

/* Set the user ID of the calling process to UID.
   If the calling process is the super-user, set the real
   and effective user IDs, and the saved set-user-ID to UID;
   if not, the effective user ID is set to UID.  */
extern int __setuid __P ((__uid_t __uid));
extern int setuid __P ((__uid_t __uid));

#ifdef	__USE_BSD
/* Set the real user ID of the calling process to RUID,
   and the effective user ID of the calling process to EUID.  */
extern int __setreuid __P ((__uid_t __ruid, __uid_t __euid));
extern int setreuid __P ((__uid_t __ruid, __uid_t __euid));

/* Set the effective user ID of the calling process to UID.  */
extern int seteuid __P ((__uid_t __uid));
#endif /* Use BSD.  */

/* Set the group ID of the calling process to GID.
   If the calling process is the super-user, set the real
   and effective group IDs, and the saved set-group-ID to GID;
   if not, the effective group ID is set to GID.  */
extern int __setgid __P ((__gid_t __gid));
extern int setgid __P ((__gid_t __gid));

#ifdef	__USE_BSD
/* Set the real group ID of the calling process to RGID,
   and the effective group ID of the calling process to EGID.  */
extern int __setregid __P ((__gid_t __rgid, __gid_t __egid));
extern int setregid __P ((__gid_t __rgid, __gid_t __egid));

/* Set the effective group ID of the calling process to GID.  */
extern int setegid __P ((__gid_t __gid));
#endif /* Use BSD.  */


/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
extern __pid_t __fork __P ((void));
extern __pid_t fork __P ((void));

#ifdef	__USE_BSD
/* Clone the calling process, but without copying the whole address space.
   The the calling process is suspended until the the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
extern __pid_t __vfork __P ((void));
extern __pid_t vfork __P ((void));
#endif /* Use BSD. */


/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
extern char *ttyname __P ((int __fd));

/* Return 1 if FD is a valid descriptor associated
   with a terminal, zero if not.  */
extern int __isatty __P ((int __fd));
extern int isatty __P ((int __fd));


/* Make a link to FROM named TO.  */
extern int __link __P ((__const char *__from, __const char *__to));
extern int link __P ((__const char *__from, __const char *__to));

#ifdef	__USE_BSD
/* Make a symbolic link to FROM named TO.  */
extern int __symlink __P ((__const char *__from, __const char *__to));
extern int symlink __P ((__const char *__from, __const char *__to));

/* Read the contents of the symbolic link PATH into no more than
   LEN bytes of BUF.  The contents are not null-terminated.
   Returns the number of characters read, or -1 for errors.  */
extern int __readlink __P ((__const char *__path, char *__buf, size_t __len));
extern int readlink __P ((__const char *__path, char *__buf, size_t __len));
#endif /* Use BSD.  */

/* Remove the link NAME.  */
extern int __unlink __P ((__const char *__name));
extern int unlink __P ((__const char *__name));

/* Remove the directory PATH.  */
extern int __rmdir __P ((__const char *__path));
extern int rmdir __P ((__const char *__path));


/* Return the foreground process group ID of FD.  */
extern __pid_t tcgetpgrp __P ((int __fd));

/* Set the foreground process group ID of FD set PGRP_ID.  */
extern int tcsetpgrp __P ((int __fd, __pid_t __pgrp_id));


/* Return the login name of the user.  */
extern char *getlogin __P ((void));

#ifdef	__USE_BSD
/* Set the login name returned by `getlogin'.  */
extern int setlogin __P ((__const char *__name));
#endif


#ifdef	__USE_POSIX2
/* Process the arguments in ARGV (ARGC of them, minus
   the program name) for options given in OPTS.

   If `opterr' is zero, no messages are generated
   for invalid options; it defaults to 1.
   `optind' is the current index into ARGV.
   `optarg' is the argument corresponding to the current option.
   Return the option character from OPTS just read.
   Return -1 when there are no more options.
   For unrecognized options, or options missing arguments,
   `optopt' is set to the option letter, and '?' is returned.

   The OPTS string is a list of characters which are recognized option
   letters, optionally followed by colons, specifying that that letter
   takes an argument, to be placed in `optarg'.

   If a letter in OPTS is followed by two colons, its argument is optional.
   This behavior is specific to the GNU `getopt'.

   The argument `--' causes premature termination of argument scanning,
   explicitly telling `getopt' that there are no more options.

   If OPTS begins with `--', then non-option arguments
   are treated as arguments to the option '\0'.
   This behavior is specific to the GNU `getopt'.  */
extern int getopt __P ((int __argc, char *__const * __argv,
			__const char *__opts));
extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;
#endif


#ifdef	__USE_BSD

/* Put the name of the current host in no more than LEN bytes of NAME.
   The result is null-terminated if LEN is large enough for the full
   name and the terminator.  */
extern int __gethostname __P ((char *__name, size_t __len));
extern int gethostname __P ((char *__name, size_t __len));

/* Set the name of the current host to NAME, which is LEN bytes long.
   This call is restricted to the super-user.  */
extern int sethostname __P ((__const char *__name, size_t __len));

/* Return the current machine's Internet number.  */
extern long int gethostid __P ((void));

/* Set the current machine's Internet number to ID.
   This call is restricted to the super-user.  */
extern int sethostid __P ((long int __id));


/* Return the number of bytes in a page.  This is the system's page size,
   which is not necessarily the same as the hardware page size.  */
extern size_t __getpagesize __P ((void));
extern size_t getpagesize __P ((void));


/* Return the maximum number of file descriptors
   the current process could possibly have.  */
extern int __getdtablesize __P ((void));
extern int getdtablesize __P ((void));


/* Truncate FILE to LENGTH bytes.  */
extern int truncate __P ((__const char *__file, __off_t __length));

/* Truncate the file FD is open on to LENGTH bytes.  */
extern int ftruncate __P ((int __fd, __off_t __length));


/* Make all changes done to FD actually appear on disk.  */
extern int fsync __P ((int __fd));

/* Make all changes done to all files actually appear on disk.  */
extern int sync __P ((void));


/* Revoke access permissions to all processes currently communicating
   with the control terminal, and then send a SIGHUP signal to the process
   group of the control terminal.  */
extern int vhangup __P ((void));


/* Turn accounting on if NAME is an existing file.  The system will then write
   a record for each process as it terminates, to this file.  If NAME is NULL,
   turn accounting off.  This call is restricted to the super-user.  */
extern int acct __P ((__const char *__name));

/* Make PATH be the root directory (the starting point for absolute paths).
   This call is restricted to the super-user.  */
extern int chroot __P ((__const char *__path));

/* Make the block special device PATH available to the system for swapping.
   This call is restricted to the super-user.  */
extern int swapon __P ((__const char *__path));

/* Reboot or halt the system.  */
extern int reboot __P ((int __howto));


/* Successive calls return the shells listed in `/etc/shells'.  */
extern char *getusershell __P ((void));
extern void endusershell __P ((void)); /* Discard cached info.  */
extern void setusershell __P ((void)); /* Rewind and re-read the file.  */


/* Prompt with PROMPT and read a string from the terminal without echoing.
   Uses /dev/tty if possible; otherwise stderr and stdin.  */
extern char *getpass __P ((const char *__prompt));

/* Put the program in the background, and dissociate from the controlling
   terminal.  If NOCHDIR is zero, do `chdir ("/")'.  If NOCLOSE is zero,
   redirects stdin, stdout, and stderr to /dev/null.  */
extern int daemon __P ((int __nochdir, int __noclose));

#endif /* Use BSD.  */


#ifdef __USE_MISC

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the file name unique.
   Returns TEMPLATE, or a null pointer if it cannot get a unique file name.  */
extern char *mktemp __P ((char *__template));

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.
   Returns a file descriptor open on the file for reading and writing,
   or -1 if it cannot create a uniquely-named file.  */
extern int mkstemp __P ((char *__template));


/* Invoke `system call' number SYSNO, passing it the remaining arguments.
   This is completely system-dependent, and not often useful.

   In Unix, `syscall' sets `errno' for all errors and most calls return -1
   for errors; in many systems you cannot pass arguments or get return
   values for all system calls (`pipe', `fork', and `getppid' typically
   among them).

   In Mach, all system calls take normal arguments and always return an
   error code (zero for success).  */
extern int syscall __P ((int __sysno, ...));

#endif	/* Use misc.  */


#if defined (__USE_MISC) && !defined (F_LOCK)
/* NOTE: These declarations also appear in <fcntl.h>; be sure to keep both
   files consistent.  Some systems have them there and some here, and some
   software depends on the macros being defined without including both.  */

/* `lockf' is a simpler interface to the locking facilities of `fcntl'.
   LEN is always relative to the current file position.
   The CMD argument is one of the following.  */

#define F_ULOCK 0       /* Unlock a previously locked region.  */
#define F_LOCK  1       /* Lock a region for exclusive use.  */ 
#define F_TLOCK 2       /* Test and lock a region for exclusive use.  */
#define F_TEST  3       /* Test a region for other processes locks.  */

extern int lockf __P ((int __fd, int __cmd, __off_t __len));
#endif /* Use misc and F_LOCK not already defined.  */


#ifdef __USE_GNU

/* Evaluate EXPRESSION, and repeat as long as it returns -1 with `errno'
   set to EINTR.  */

#define TEMP_FAILURE_RETRY(expression) \
  ({ long int __result;							      \
       do __result = (long int) (expression);				      \
       while (__result == -1L && errno == EINTR);			      \
       __result; })
          
#endif

__END_DECLS

#endif /* unistd.h  */
