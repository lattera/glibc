#ifndef	_PRINTF_H

#include <stdio-common/printf.h>

/* Now define the internal interfaces.  */
extern int __printf_fphex (FILE *, const struct printf_info *,
			   const void *const *);
extern int __printf_fp (FILE *, const struct printf_info *,
			const void *const *);
libc_hidden_proto (__printf_fp)

#endif
