#ifndef _ALLOCA_H

#include <stdlib/alloca.h>
#include <stackinfo.h>

#undef	__alloca

/* Now define the internal interfaces.  */
extern void *__alloca (size_t __size);

#ifdef	__GNUC__
# define __alloca(size)	__builtin_alloca (size)
#endif /* GCC.  */

extern int __libc_use_alloca (size_t size) __attribute__ ((const));
extern int __libc_alloca_cutoff (size_t size) __attribute__ ((const));
libc_hidden_proto (__libc_alloca_cutoff)

#define __MAX_ALLOCA_CUTOFF	65536

#include <allocalim.h>

#ifndef stackinfo_alloca_round
# define stackinfo_alloca_round(l) (((l) + 15) & -16)
#endif

#if _STACK_GROWS_DOWN
# define extend_alloca(buf, len, newlen) \
  (__typeof (buf)) ({ size_t __newlen = stackinfo_alloca_round (newlen);      \
		      char *__newbuf = __alloca (__newlen);		      \
		      if (__newbuf + __newlen == (char *) buf)		      \
			len += __newlen;				      \
		      else						      \
			len = __newlen;					      \
		      __newbuf; })
#elif _STACK_GROWS_UP
# define extend_alloca(buf, len, newlen) \
  (__typeof (buf)) ({ size_t __newlen = stackinfo_alloca_round (newlen);      \
		      char *__newbuf = __alloca (__newlen);		      \
		      char *__buf = (buf);				      \
		      if (__buf + len == __newbuf)			      \
			{						      \
			  len += __newlen;				      \
			  __newbuf = __buf;				      \
			}						      \
		      else						      \
			len = __newlen;					      \
		      __newbuf; })
#else
# define extend_alloca(buf, len, newlen) \
  __alloca (((len) = (newlen)))
#endif

#if defined stackinfo_get_sp && defined stackinfo_sub_sp
# define alloca_account(size, avar) \
  ({ void *old__ = stackinfo_get_sp ();					      \
     void *m__ = __alloca (size);					      \
     avar += stackinfo_sub_sp (old__);					      \
     m__; })
# define extend_alloca_account(buf, len, newlen, avar) \
  ({ void *old__ = stackinfo_get_sp ();					      \
     void *m__ = extend_alloca (buf, len, newlen);			      \
     avar += stackinfo_sub_sp (old__);					      \
     m__; })
#else
# define alloca_account(size, avar) \
  ({ size_t s__ = (size);						      \
     avar += s__;							      \
     __alloca (s__); })
# define extend_alloca_account(buf, len, newlen, avar) \
  ({ size_t s__ = (newlen);						      \
     avar += s__;							      \
     extend_alloca (buf, len, s__); })
#endif

#endif
