/* Sparc is big-endian, but v9 supports endian conversion on loads/stores
   and GCC supports such a mode.  Be prepared.  */

#ifdef __LITTLE_ENDIAN__
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#define __BYTE_ORDER __BIG_ENDIAN
#endif
