/* Set endianness for tile.  */

#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly; include <endian.h> instead."
#endif

#if defined __BIG_ENDIAN__
# define __BYTE_ORDER __BIG_ENDIAN
#elif defined __LITTLE_ENDIAN__
# define __BYTE_ORDER __LITTLE_ENDIAN
#else
# error "Endianness not declared!!"
#endif
