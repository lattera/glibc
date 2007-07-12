#include <resolv/arpa/nameser.h>

/* If the machine allows unaligned access we can do better than using
   the NS_GET16, NS_GET32, NS_PUT16, and NS_PUT32 macros from the
   installed header.  */
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>

extern struct _ns_flagdata _ns_flagdata[] attribute_hidden;

#if _STRING_ARCH_unaligned

# undef NS_GET16
# define NS_GET16(s, cp) \
  do {									      \
    uint16_t *t_cp = (uint16_t *) (cp);					      \
    (s) = ntohs (*t_cp);						      \
    (cp) += NS_INT16SZ;							      \
  } while (0)

# undef NS_GET32
# define NS_GET32(l, cp) \
  do {									      \
    uint32_t *t_cp = (uint32_t *) (cp);					      \
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
