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
    char d_name[1];		/* Actually longer. */
  };

#include <stddef.h>

/* We calculate the length of the name by taking the length of the whole
   `struct direct' record, subtracting the size of everything before the
   name, and subtracting one for the terminating null.  */

#define D_NAMLEN(d) \
  ((d)->d_reclen - offsetof (struct direct, d_name) - 1)
