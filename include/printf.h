#ifndef	_PRINTF_H

#include <stdio-common/printf.h>

# ifndef _ISOMAC

#include <xlocale.h>

/* Now define the internal interfaces.  */
extern int __printf_fphex (FILE *, const struct printf_info *,
			   const void *const *);
extern int __printf_fp (FILE *, const struct printf_info *,
			const void *const *);
libc_hidden_proto (__printf_fp)
extern int __printf_fp_l (FILE *, locale_t, const struct printf_info *,
			  const void *const *);
libc_hidden_proto (__printf_fp_l)

# endif /* !_ISOMAC */
#endif
