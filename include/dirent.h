#ifndef _DIRENT_H
# include <dirstream.h>
# include <dirent/dirent.h>
# include <sys/stat.h>
# include <stdbool.h>

struct scandir_cancel_struct
{
  DIR *dp;
  void *v;
  size_t cnt;
};

/* Now define the internal interfaces.  */
extern DIR *__opendir (__const char *__name);
extern DIR *__opendirat (int dfd, __const char *__name) internal_function;
extern DIR *__fdopendir (int __fd);
extern int __closedir (DIR *__dirp);
extern struct dirent *__readdir (DIR *__dirp);
extern struct dirent64 *__readdir64 (DIR *__dirp);
extern int __readdir_r (DIR *__dirp, struct dirent *__entry,
			struct dirent **__result);
extern int __readdir64_r (DIR *__dirp, struct dirent64 *__entry,
			  struct dirent64 **__result);
extern int __scandir64 (__const char * __dir,
			struct dirent64 *** __namelist,
			int (*__selector) (__const struct dirent64 *),
			int (*__cmp) (__const struct dirent64 **,
				      __const struct dirent64 **));
extern __ssize_t __getdents (int __fd, char *__buf, size_t __nbytes)
     internal_function;
extern __ssize_t __getdents64 (int __fd, char *__buf, size_t __nbytes)
     internal_function;
extern int __alphasort64 (const struct dirent64 **a, const struct dirent64 **b)
     __attribute_pure__;
extern int __versionsort64 (const struct dirent64 **a,
			    const struct dirent64 **b)
     __attribute_pure__;
extern DIR *__alloc_dir (int fd, bool close_fd, int flags,
			 const struct stat64 *statp)
     internal_function;
extern void __scandir_cancel_handler (void *arg);

libc_hidden_proto (rewinddir)
libc_hidden_proto (scandirat)
libc_hidden_proto (scandirat64)

#endif
