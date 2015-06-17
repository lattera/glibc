#ifndef	_TTYENT_H
# include <misc/ttyent.h>

extern __typeof (getttyent) __getttyent __THROW;
libc_hidden_proto (__getttyent)
extern __typeof (setttyent) __setttyent __THROW;
libc_hidden_proto (__setttyent)
extern __typeof (endttyent) __endttyent __THROW;
libc_hidden_proto (__endttyent)

#endif
