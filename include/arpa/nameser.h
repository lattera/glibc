#ifndef _ARPA_NAMESER_H_

#include <resolv/arpa/nameser.h>

# ifndef _ISOMAC

/* If the machine allows unaligned access we can do better than using
   the NS_GET16, NS_GET32, NS_PUT16, and NS_PUT32 macros from the
   installed header.  */
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>

extern const struct _ns_flagdata _ns_flagdata[] attribute_hidden;

#if _STRING_ARCH_unaligned

# undef NS_GET16
# define NS_GET16(s, cp) \
  do {									      \
    const uint16_t *t_cp = (const uint16_t *) (cp);			      \
    (s) = ntohs (*t_cp);						      \
    (cp) += NS_INT16SZ;							      \
  } while (0)

# undef NS_GET32
# define NS_GET32(l, cp) \
  do {									      \
    const uint32_t *t_cp = (const uint32_t *) (cp);			      \
    (l) = ntohl (*t_cp);						      \
    (cp) += NS_INT32SZ;							      \
  } while (0)

# undef NS_PUT16
# define NS_PUT16(s, cp) \
  do {									      \
    uint16_t *t_cp = (uint16_t *) (cp);					      \
    *t_cp = htons (s);							      \
    (cp) += NS_INT16SZ;							      \
  } while (0)

# undef NS_PUT32
# define NS_PUT32(l, cp) \
  do {									      \
    uint32_t *t_cp = (uint32_t *) (cp);					      \
    *t_cp = htonl (l);							      \
    (cp) += NS_INT32SZ;							      \
  } while (0)

#endif

extern unsigned int	__ns_get16 (const unsigned char *) __THROW;
extern unsigned long	__ns_get32 (const unsigned char *) __THROW;

#define ns_msg_getflag(handle, flag) \
  (((handle)._flags & _ns_flagdata[flag].mask) >> _ns_flagdata[flag].shift)

libresolv_hidden_proto (ns_get16)
libresolv_hidden_proto (ns_get32)
libresolv_hidden_proto (ns_put16)
libresolv_hidden_proto (ns_put32)
libresolv_hidden_proto (ns_initparse)
libresolv_hidden_proto (ns_skiprr)
libresolv_hidden_proto (ns_parserr)
libresolv_hidden_proto (ns_name_ntop)
libresolv_hidden_proto (ns_name_pton)
libresolv_hidden_proto (ns_name_pack)
libresolv_hidden_proto (ns_name_skip)
libresolv_hidden_proto (ns_name_unpack)
libresolv_hidden_proto (ns_name_compress)
libresolv_hidden_proto (ns_name_uncompress)
libresolv_hidden_proto (ns_sprintrr)
libresolv_hidden_proto (ns_sprintrrf)
libresolv_hidden_proto (ns_samedomain)
libresolv_hidden_proto (ns_samename)
libresolv_hidden_proto (ns_makecanon)
libresolv_hidden_proto (ns_format_ttl)

# endif /* !_ISOMAC */
#endif
