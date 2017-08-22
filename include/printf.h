#ifndef	_PRINTF_H

#include <stdio-common/printf.h>

# ifndef _ISOMAC

#include <bits/types/locale_t.h>

/* Now define the internal interfaces.  */
extern int __printf_fphex (FILE *, const struct printf_info *,
			   const void *const *) attribute_hidden;
extern int __printf_fp (FILE *, const struct printf_info *,
			const void *const *);
libc_hidden_proto (__printf_fp)
extern int __printf_fp_l (FILE *, locale_t, const struct printf_info *,
			  const void *const *);
libc_hidden_proto (__printf_fp_l)

extern unsigned int __guess_grouping (unsigned int intdig_max,
				      const char *grouping)
     attribute_hidden;

# endif /* !_ISOMAC */
#endif
