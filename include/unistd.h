#ifndef _UNISTD_H
# include <posix/unistd.h>

/* Now define the internal interfaces.  */
extern int __access (__const char *__name, int __type);
extern int __euidaccess (__const char *__name, int __type);
extern __off64_t __lseek64 (int __fd, __off64_t __offset, int __whence);
extern __off_t __lseek (int __fd, __off_t __offset, int __whence);
extern __off_t __libc_lseek (int __fd, __off_t __offset, int __whence);
extern __off64_t __libc_lseek64 (int __fd, __off64_t __offset, int __whence);
extern ssize_t __pread (int __fd, void *__buf, size_t __nbytes,
			__off_t __offset);
extern ssize_t __libc_pread (int __fd, void *__buf, size_t __nbytes,
			     __off_t __offset);
extern ssize_t __pread64 (int __fd, void *__buf, size_t __nbytes,
			  __off64_t __offset);
extern ssize_t __libc_pread64 (int __fd, void *__buf, size_t __nbytes,
			       __off64_t __offset);
extern ssize_t __pwrite (int __fd, __const void *__buf, size_t __n,
			 __off_t __offset);
extern ssize_t __libc_pwrite (int __fd, __const void *__buf, size_t __n,
			      __off_t __offset);
extern ssize_t __pwrite64 (int __fd, __const void *__buf, size_t __n,
			   __off64_t __offset);
extern ssize_t __libc_pwrite64 (int __fd, __const void *__buf, size_t __n,
				__off64_t __offset);
extern ssize_t __libc_read (int __fd, void *__buf, size_t __n);
extern ssize_t __libc_write (int __fd, __const void *__buf, size_t __n);
extern int __pipe (int __pipedes[2]);
extern unsigned int __sleep (unsigned int __seconds);
extern int __chown (__const char *__file,
		    __uid_t __owner, __gid_t __group);
extern int __fchown (int __fd,
		     __uid_t __owner, __gid_t __group);
extern int __lchown (__const char *__file, __uid_t __owner,
		     __gid_t __group);
extern int __chdir (__const char *__path);
extern int __fchdir (int __fd);
extern char *__getcwd (char *__buf, size_t __size);
extern int __rmdir (const char *__path);

/* Get the canonical absolute name of the named directory, and put it in SIZE
   bytes of BUF.  Returns NULL if the directory couldn't be determined or
   SIZE was too small.  If successful, returns BUF.  In GNU, if BUF is
   NULL, an array is allocated with `malloc'; the array is SIZE bytes long,
   unless SIZE <= 0, in which case it is as big as necessary.  */

char *__canonicalize_directory_name_internal (__const char *__thisdir,
					      char *__buf,
					      size_t __size);

extern int __dup (int __fd);
extern int __dup2 (int __fd, int __fd2);
extern int __execve (__const char *__path, char *__const __argv[],
		     char *__const __envp[]);
extern long int __pathconf (__const char *__path, int __name);
extern long int __fpathconf (int __fd, int __name);
extern long int __sysconf (int __name);
extern __pid_t __getpid (void);
extern __pid_t __getppid (void);
extern __pid_t __setsid (void);
extern __uid_t __getuid (void);
extern __uid_t __geteuid (void);
extern __gid_t __getgid (void);
extern __gid_t __getegid (void);
extern int __getgroups (int __size, __gid_t __list[]);
extern int __group_member (__gid_t __gid);
extern int __setuid (__uid_t __uid);
extern int __setreuid (__uid_t __ruid, __uid_t __euid);
extern int __setgid (__gid_t __gid);
extern int __setpgid (__pid_t __pid, __pid_t __pgid);
extern int __setregid (__gid_t __rgid, __gid_t __egid);
extern __pid_t __vfork (void);
extern int __ttyname_r (int __fd, char *__buf, size_t __buflen);
extern int __isatty (int __fd);
extern int __link (__const char *__from, __const char *__to);
extern int __symlink (__const char *__from, __const char *__to);
extern int __readlink (__const char *__path, char *__buf, size_t __len);
extern int __unlink (__const char *__name);
extern int __gethostname (char *__name, size_t __len);
extern int __profil (unsigned short int *__sample_buffer, size_t __size,
		     size_t __offset, unsigned int __scale);
extern int __getdtablesize (void);
extern int __brk (void *__addr);
extern int __close (int __fd);
extern ssize_t __read (int __fd, void *__buf, size_t __nbytes);
extern ssize_t __write (int __fd, __const void *__buf, size_t __n);
extern __pid_t __fork (void);
extern int __getpagesize (void) __attribute__ ((__const__));
extern int __ftruncate (int __fd, __off_t __length);
extern int __ftruncate64 (int __fd, __off64_t __length);
extern void *__sbrk (intptr_t __delta);


/* This variable is set nonzero at startup if the process's effective
   IDs differ from its real IDs, or it is otherwise indicated that
   extra security should be used.  When this is set the dynamic linker
   and some functions contained in the C library ignore various
   environment variables that normally affect them.  */
extern int __libc_enable_secure;


/* Various internal function.  */
extern void __libc_check_standard_fds (void);


#endif
