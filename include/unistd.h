#ifndef _UNISTD_H
# include <posix/unistd.h>

/* Now define the internal interfaces.  */
extern int __access __P ((__const char *__name, int __type));
extern int __euidaccess __P ((__const char *__name, int __type));
extern __off64_t __lseek64 __P ((int __fd, __off64_t __offset, int __whence));
extern ssize_t __pread __P ((int __fd, __ptr_t __buf, size_t __nbytes,
			     __off_t __offset));
extern ssize_t __pwrite __P ((int __fd, __const __ptr_t __buf, size_t __n,
			      __off_t __offset));
extern int __pipe __P ((int __pipedes[2]));
extern unsigned int __sleep __P ((unsigned int __seconds));
extern int __chown __P ((__const char *__file,
			 __uid_t __owner, __gid_t __group));
extern int __fchown __P ((int __fd,
			  __uid_t __owner, __gid_t __group));
extern int __lchown __P ((__const char *__file, __uid_t __owner,
			  __gid_t __group));
extern int __chdir __P ((__const char *__path));
extern int __fchdir __P ((int __fd));
extern char *__getcwd __P ((char *__buf, size_t __size));

/* Get the canonical absolute name of the named directory, and put it in SIZE
   bytes of BUF.  Returns NULL if the directory couldn't be determined or
   SIZE was too small.  If successful, returns BUF.  In GNU, if BUF is
   NULL, an array is allocated with `malloc'; the array is SIZE bytes long,
   unless SIZE <= 0, in which case it is as big as necessary.  */

char *__canonicalize_directory_name_internal __P ((__const char *__thisdir,
						   char *__buf,
						   size_t __size));

extern int __dup __P ((int __fd));
extern int __dup2 __P ((int __fd, int __fd2));
extern int __execve __P ((__const char *__path, char *__const __argv[],
			  char *__const __envp[]));
extern long int __pathconf __P ((__const char *__path, int __name));
extern long int __fpathconf __P ((int __fd, int __name));
extern long int __sysconf __P ((int __name));
extern __pid_t __getppid __P ((void));
extern __pid_t __setsid __P ((void));
extern __uid_t __getuid __P ((void));
extern __uid_t __geteuid __P ((void));
extern __gid_t __getgid __P ((void));
extern __gid_t __getegid __P ((void));
extern int __getgroups __P ((int __size, __gid_t __list[]));
extern int __group_member __P ((__gid_t __gid));
extern int __setuid __P ((__uid_t __uid));
extern int __setreuid __P ((__uid_t __ruid, __uid_t __euid));
extern int __setgid __P ((__gid_t __gid));
extern int __setregid __P ((__gid_t __rgid, __gid_t __egid));
extern __pid_t __vfork __P ((void));
extern int __ttyname_r __P ((int __fd, char *__buf, size_t __buflen));
extern int __isatty __P ((int __fd));
extern int __link __P ((__const char *__from, __const char *__to));
extern int __symlink __P ((__const char *__from, __const char *__to));
extern int __readlink __P ((__const char *__path, char *__buf, size_t __len));
extern int __unlink __P ((__const char *__name));
extern int __rmdir __P ((__const char *__path));
extern int __gethostname __P ((char *__name, size_t __len));
extern int __profil __P ((unsigned short int *__sample_buffer, size_t __size,
			  size_t __offset, unsigned int __scale));
extern int __getdtablesize __P ((void));
extern int __brk __P ((__ptr_t __addr));
#endif
