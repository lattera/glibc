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
