#ifndef	_STDIO_EXT_H

# include <stdio-common/stdio_ext.h>

extern int __fsetlocking_internal (FILE *__fp, int __type) attribute_hidden;

#define __fsetlocking(fp, type) \
  ({ int __result = ((fp->_flags & _IO_USER_LOCK)			\
		     ? FSETLOCKING_BYCALLER : FSETLOCKING_INTERNAL);	\
									\
     if (type != FSETLOCKING_QUERY)					\
       {								\
	 fp->_flags &= ~_IO_USER_LOCK;					\
	 if (type == FSETLOCKING_BYCALLER)				\
	   fp->_flags |= _IO_USER_LOCK;					\
       }								\
									\
     __result;								\
  })


#endif
