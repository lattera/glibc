#ifndef _ELF_H
#include <elf/elf.h>

# ifndef _ISOMAC

/* Some information which is not meant for the public and therefore not
   in <elf.h>.  */
# include <dl-dtprocnum.h>
# ifdef DT_1_SUPPORTED_MASK
#  error DT_1_SUPPORTED_MASK is defined!
# endif
# define DT_1_SUPPORTED_MASK \
   (DF_1_NOW | DF_1_NODELETE | DF_1_INITFIRST | DF_1_NOOPEN \
    | DF_1_ORIGIN | DF_1_NODEFLIB)

# endif /* !_ISOMAC */
#endif /* elf.h */
