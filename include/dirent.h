#ifndef _DIRENT_H
# include <dirent/dirent.h>

/* Now define the internal interfaces.  */
extern DIR *__opendir (__const char *__name);
extern int __closedir (DIR *__dirp);
extern struct dirent *__readdir (DIR *__dirp);
extern struct dirent64 *__readdir64 (DIR *__dirp);
extern int __readdir_r (DIR *__dirp, struct dirent *__entry,
			struct dirent **__result);
extern __ssize_t __getdents (int __fd, char *__buf, size_t __nbytes)
     internal_function;
extern __ssize_t __getdents64 (int __fd, char *__buf, size_t __nbytes)
     internal_function;
#endif
