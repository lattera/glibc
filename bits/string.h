/* This file should provide inline versions of string functions.

   Surround GCC-specific parts with #ifdef __GNUC__, and use `__extern_inline'.

   This file should define __STRING_INLINES if functions are actually defined
   as inlines.  */

#ifndef _BITS_STRING_H
#define _BITS_STRING_H	1

/* Define whether to use the unaligned string inline ABI.
   The string inline functions are an external ABI, thus cannot be changed
   after the first release of a new target (unlike _STRING_ARCH_unaligned
   which may be changed from release to release).  Targets must support
   unaligned accesses in hardware if either define is set to true.  */
#define _STRING_INLINE_unaligned   0

#endif /* bits/string.h */
