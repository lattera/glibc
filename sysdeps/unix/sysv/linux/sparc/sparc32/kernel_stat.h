/* Definition of `struct stat' used in the kernel */
struct kernel_stat
  {
    unsigned short int st_dev;
    unsigned long int st_ino;
    unsigned short int st_mode;
    short int st_nlink;
    unsigned short int st_uid;
    unsigned short int st_gid;
    unsigned short int st_rdev;
    long int st_size;
    long int st_atime;
    unsigned long int __unused1;
    long int st_mtime;
    unsigned long int __unused2;
    long int st_ctime;
    unsigned long int __unused3;
    long int st_blksize;
    long int st_blocks;
    unsigned long int __unused4;
    unsigned long int __unused5;
  };

#define _HAVE___UNUSED1
#define _HAVE___UNUSED2
#define _HAVE___UNUSED3
#define _HAVE___UNUSED4
#define _HAVE___UNUSED5

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
#define _HAVE_STAT64___PAD2
