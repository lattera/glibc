/* Definition of `struct stat' used in the kernel..  */
struct kernel_stat
  {
    unsigned long int st_dev;
    long int __pad1[3];			/* Reserved for network id */
    unsigned long int st_ino;
    unsigned long int st_mode;
    unsigned long int st_nlink;
    long int st_uid;
    long int st_gid;
    unsigned long int st_rdev;
    long int __pad2[2];
    long int st_size;
    long int __pad3;
    long int st_atime;
    long int __unused1;
    long int st_mtime;
    long int __unused2;
    long int st_ctime;
    long int __unused3;
    long int st_blksize;
    long int st_blocks;
    char st_fstype[16];			/* Filesystem type name, unsupported */
    long st_pad4[8];
    /* Linux specific fields */
    unsigned int st_flags;
    unsigned int st_gen;
  };
