#ifndef	MAXNAMLEN
#define	MAXNAMLEN	512
#endif
#define DIRBUF	        1048	/* minimum buffer size for call to getdents */

struct direct
  {
    unsigned short int d_fileno;
    short int d_pad;
    long int d_off;
    unsigned short int d_reclen;
    char d_name[1];
  };
