#ifndef	MAXNAMLEN
#define	MAXNAMLEN	255
#endif

struct direct
  {
    unsigned int d_fileno;	/* 32 bits.  */
    unsigned short int d_reclen; /* 16 bits.  */
    unsigned short int d_namlen; /* 16 bits.  */
    char d_name[MAXNAMLEN + 1];
  };

#define D_NAMLEN(d) ((d)->d_namlen)
