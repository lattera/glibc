/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* ...but edited by hand to be used in GNU libc.  */
#include <sys/stat.h>		/* To get _STATBUF_ST_BLKSIZE.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if your struct stat has st_blksize.  */
#ifdef _STATBUF_ST_BLKSIZE
# define HAVE_ST_BLKSIZE 1
#endif

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
/* #undef STAT_MACROS_BROKEN */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Say we have endian.h.  */
#define HAVE_ENDIAN_H	1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef ssize_t */

/* Define if you want a debugging version. */
/* #undef DEBUG */

/* Define if you want a version with run-time diagnostic checking. */
/* #undef DIAGNOSTIC */

/* Define if you have sigfillset (and sigprocmask). */
#define HAVE_SIGFILLSET 1

/* Define if building on AIX, HP, Solaris to get big-file environment. */
/* #undef HAVE_FILE_OFFSET_BITS */
#ifdef HAVE_FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

/* Define if you have spinlocks. */
/* #undef HAVE_SPINLOCKS */

/* Define if you want to use mc68020/gcc assembly spinlocks. */
/* #undef HAVE_ASSEM_MC68020_GCC */

/* Define if you want to use parisc/gcc assembly spinlocks. */
/* #undef HAVE_ASSEM_PARISC_GCC */

/* Define if you want to use sco/cc assembly spinlocks. */
/* #undef HAVE_ASSEM_SCO_CC */

/* Define if you want to use sparc/gcc assembly spinlocks. */
/* #undef HAVE_ASSEM_SPARC_GCC */

/* Define if you want to use uts4/cc assembly spinlocks. */
/* #undef HAVE_ASSEM_UTS4_CC */

/* Define if you want to use x86/gcc assembly spinlocks. */
/* #undef HAVE_ASSEM_X86_GCC */

/* Define if you have the AIX _check_lock spinlocks. */
/* #undef HAVE_FUNC_AIX */

/* Define if you have the OSF1 or HPPA msemaphore spinlocks. */
/* #undef HAVE_FUNC_MSEM */

/* Define if you have the SGI abilock_t spinlocks. */
/* #undef HAVE_FUNC_SGI */

/* Define if you have the ReliantUNIX spinlock_t spinlocks. */
/* #undef HAVE_FUNC_RELIANT */

/* Define if you have the Solaris mutex_t spinlocks. */
/* #undef HAVE_FUNC_SOLARIS */

/* Define if your sprintf returns a pointer, not a length. */
/* #undef SPRINTF_RET_CHARPNT */

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getopt function.  */
#define HAVE_GETOPT 1

/* Define if you have the getuid function.  */
#define HAVE_GETUID 1

/* Define if you have the memcmp function.  */
#define HAVE_MEMCMP 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the mmap function.  */
#define HAVE_MMAP 1

/* Define if you have the raise function.  */
#define HAVE_RAISE 1

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the shmget function.  */
#define HAVE_SHMGET 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strsep function.  */
#define HAVE_STRSEP 1

/* Define if you have the sysconf function.  */
#define HAVE_SYSCONF 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/select.h> header file.  */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

#include_next <config.h>
