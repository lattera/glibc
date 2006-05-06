#include <resolv/arpa/nameser.h>

/* If the machine allows unaligned access we can do better than using
   the NS_GET16, NS_GET32, NS_PUT16, and NS_PUT32 macros from the
   installed header.  */
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>

#if _STRING_ARCH_unaligned

# undef NS_GET16
# define NS_GET16(s, cp) \
  do {									      \
    uint16_t *t_cp = (uint16_t *) (cp);					      \
    (s) = ntohs (*t_cp);						      \
    (cp) += NS_INT16SZ;							      \
  } while (0)

# undef NS_GET32
# define NS_GET32(s, cp) \
  do {									      \
    uint32_t *t_cp = (uint32_t *) (cp);					      \
    (s) = ntohl (*t_cp);						      \
    (cp) += NS_INT32SZ;							      \
  } while (0)

#endif
