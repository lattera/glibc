#ifndef __DES_CRYPT_H__
#include <sunrpc/rpc/des_crypt.h>

/* Now define the internal interfaces.  */

extern int xencrypt (char *secret, char *passwd);
extern int xdecrypt (char *secret, char *passwd);

#endif
