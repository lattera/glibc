/* The MIPS architecture has selectable endianness.
   The DECstation uses little-endian mode.  */

#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly; include <endian.h> instead."
#endif

#define __BYTE_ORDER __LITTLE_ENDIAN
