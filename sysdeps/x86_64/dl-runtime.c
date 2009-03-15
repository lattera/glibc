/* The ABI calls for the PLT stubs to pass the index of the relocation
   and not its offset.  In _dl_profile_fixup and _dl_call_pltexit we
   also use the index.  Therefore it is wasteful to compute the offset
   in the trampoline just to reverse the operation immediately
   afterwards.  */
#define reloc_offset reloc_arg * sizeof (PLTREL)
#define reloc_index  reloc_arg

#include <elf/dl-runtime.c>
