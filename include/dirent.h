#ifndef _DIRENT_H
# include <dirent/dirent.h>

/* Now define the internal interfaces.  */
extern DIR *__opendir (__const char *__name) __THROW;
extern int __closedir (DIR *__dirp) __THROW;
extern struct dirent *__readdir (DIR *__dirp) __THROW;
extern struct dirent64 *__readdir64 (DIR *__dirp) __THROW;
extern int __readdir_r (DIR *__dirp, struct dirent *__entry,
			struct dirent **__result) __THROW;
extern __ssize_t __getdents (int __fd, char *__buf, size_t __nbytes)
     internal_function;
extern __ssize_t __getdents64 (int __fd, char *__buf, size_t __nbytes)
     internal_function;
#endif
