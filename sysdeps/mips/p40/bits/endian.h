/* The MIPS has selectable endianness.
   The Japanese homebrew P40 architecture uses big-endian mode.  */

#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly; include <endian.h> instead."
#endif

#define __BYTE_ORDER __BIG_ENDIAN
