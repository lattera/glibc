/* Definition of `struct stat' used in the kernel */
struct kernel_stat
  {
    unsigned int st_dev;
    unsigned long int st_ino;
    unsigned int st_mode;
    short int st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned int st_rdev;
    long int st_size;
    long int st_atime;
    long int st_mtime;
    long int st_ctime;
    long int st_blksize;
    long int st_blocks;
    unsigned long int __unused1;
    unsigned long int __unused2;
  };

#define _HAVE___UNUSED1
#define _HAVE___UNUSED2

#define _HAVE_STAT___UNUSED1
#define _HAVE_STAT___UNUSED2
#define _HAVE_STAT___UNUSED3
#define _HAVE_STAT___UNUSED4
#define _HAVE_STAT___UNUSED5
#define _HAVE_STAT___PAD1
#define _HAVE_STAT___PAD2
#define _HAVE_STAT64___UNUSED1
#define _HAVE_STAT64___UNUSED2
#define _HAVE_STAT64___UNUSED3
#define _HAVE_STAT64___UNUSED4
#define _HAVE_STAT64___UNUSED5
#define _HAVE_STAT64___PAD1
#define _HAVE_STAT64___PAD2

#define XSTAT_IS_XSTAT64 1
