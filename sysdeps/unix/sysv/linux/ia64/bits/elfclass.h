/* This file specifies the native word size of the machine, which indicates
   the ELF file class used for executables and shared objects on this
   machine.  */

#ifndef _LINK_H
# error "Never use <bits/elfclass.h> directly; include <link.h> instead."
#endif

#include <bits/wordsize.h>

#define __ELF_NATIVE_CLASS __WORDSIZE

/* Linux/ia64, like the Alpha has the .hash section with 64 bit entries.  */
typedef uint64_t Elf_Symndx;
