/* Definition of `struct stat' used in the kernel..  */
struct kernel_stat
  {
    unsigned short int st_dev;
    unsigned short int __pad1;
#define _HAVE___PAD1
    unsigned long int st_ino;
    unsigned short int st_mode;
    unsigned short int st_nlink;
    unsigned short int st_uid;
    unsigned short int st_gid;
    unsigned short int st_rdev;
    unsigned short int __pad2;
#define _HAVE___PAD2
    unsigned long int st_size;
    unsigned long int st_blksize;
    unsigned long int st_blocks;
    unsigned long int st_atime;
    unsigned long int __unused1;
#define _HAVE___UNUSED1
    unsigned long int st_mtime;
    unsigned long int __unused2;
#define _HAVE___UNUSED2
    unsigned long int st_ctime;
    unsigned long int __unused3;
#define _HAVE___UNUSED3
    unsigned long int __unused4;
#define _HAVE___UNUSED4
    unsigned long int __unused5;
#define _HAVE___UNUSED5
  };
