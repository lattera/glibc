#ifndef	MAXNAMLEN
#define	MAXNAMLEN	255
#endif

struct direct
  {
    unsigned long int d_ino;
    off_t d_off;
    unsigned short int d_reclen;
    char d_name[MAXNAMLEN + 1];
  };

#define D_NAMLEN(d) (strlen ((d)->d_name))

#define D_RECLEN(d) (d->d_reclen)
