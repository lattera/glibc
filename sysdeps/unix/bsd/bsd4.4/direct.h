#ifndef	MAXNAMLEN
#define	MAXNAMLEN	255
#endif

struct direct
  {
    unsigned long int d_fileno;
    unsigned short int d_reclen;
    unsigned char d_type;	/* File type, possibly unknown.  */
    unsigned char d_namlen;	/* Length of the file name.  */
    char d_name[MAXNAMLEN + 1];
  };

#define D_NAMLEN(d) ((d)->d_namlen)

#define HAVE_D_TYPE
