struct direct
  {
    unsigned short int d_fileno;
    char d_name[14];
  };

#define D_NAMLEN(d) \
  ((d)->d_name[13] == '\0' ? strlen ((d)->d_name) : 14)

#define D_RECLEN(d) (sizeof (*(d)))
