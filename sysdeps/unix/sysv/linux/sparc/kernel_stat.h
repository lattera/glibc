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
    unsigned long int __unused4[2];
  };
