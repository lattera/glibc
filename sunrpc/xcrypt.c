/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * Copyright (c) 1986-1991 by Sun Microsystems Inc.
 */

#if 0
#ident	"@(#)xcrypt.c	1.11	94/08/23 SMI"
#endif

#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)xcrypt.c 1.3 89/03/24 Copyr 1986 Sun Micro";
#endif

/*
 * xcrypt.c: Hex encryption/decryption and utility routines
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <rpc/des_crypt.h>

static const char hex[16] =
{
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};


#ifdef _LIBC
# define hexval(c) \
  (c >= '0' && c <= '9'							      \
   ? c - '0'								      \
   : ({	int upp = toupper (c);						      \
	upp >= 'A' && upp <= 'Z' ? upp - 'A' + 10 : -1; }))
#else
static char hexval (char) internal_function;
#endif

static void hex2bin (int, char *, char *) internal_function;
static void bin2hex (int, unsigned char *, char *) internal_function;
void passwd2des_internal (char *pw, char *key);
#ifdef _LIBC
libc_hidden_proto (passwd2des_internal)
#endif

/*
 * Turn password into DES key
 */
void
passwd2des_internal (char *pw, char *key)
{
  int i;

  memset (key, 0, 8);
  for (i = 0; *pw && i < 8; ++i)
    key[i] ^= *pw++ << 1;

  des_setparity (key);
}

#ifdef _LIBC
libc_hidden_def (passwd2des_internal)
strong_alias (passwd2des_internal, passwd2des)
#else
void passwd2des (char *pw, char *key)
{
  return passwd2des_internal (pw, key);
}
#endif

/*
 * Encrypt a secret key given passwd
 * The secret key is passed and returned in hex notation.
 * Its length must be a multiple of 16 hex digits (64 bits).
 */
int
xencrypt (char *secret, char *passwd)
{
  char key[8];
  char ivec[8];
  char *buf;
  int err;
  int len;

  len = strlen (secret) / 2;
  buf = malloc ((unsigned) len);
  hex2bin (len, secret, buf);
  passwd2des_internal (passwd, key);
  memset (ivec, 0, 8);

  err = cbc_crypt (key, buf, len, DES_ENCRYPT | DES_HW, ivec);
  if (DES_FAILED (err))
    {
      free (buf);
      return 0;
    }
  bin2hex (len, (unsigned char *) buf, secret);
  free (buf);
  return 1;
}

/*
 * Decrypt secret key using passwd
 * The secret key is passed and returned in hex notation.
 * Once again, the length is a multiple of 16 hex digits
 */
int
xdecrypt (char *secret, char *passwd)
{
  char key[8];
  char ivec[8];
  char *buf;
  int err;
  int len;

  len = strlen (secret) / 2;
  buf = malloc ((unsigned) len);

  hex2bin (len, secret, buf);
  passwd2des_internal (passwd, key);
  memset (ivec, 0, 8);

  err = cbc_crypt (key, buf, len, DES_DECRYPT | DES_HW, ivec);
  if (DES_FAILED (err))
    {
      free (buf);
      return 0;
    }
  bin2hex (len, (unsigned char *) buf, secret);
  free (buf);
  return 1;
}

/*
 * Hex to binary conversion
 */
static void
internal_function
hex2bin (int len, char *hexnum, char *binnum)
{
  int i;

  for (i = 0; i < len; i++)
    *binnum++ = 16 * hexval (hexnum[2 * i]) + hexval (hexnum[2 * i + 1]);
}

/*
 * Binary to hex conversion
 */
static void
internal_function
bin2hex (int len, unsigned char *binnum, char *hexnum)
{
  int i;
  unsigned val;

  for (i = 0; i < len; i++)
    {
      val = binnum[i];
      hexnum[i * 2] = hex[val >> 4];
      hexnum[i * 2 + 1] = hex[val & 0xf];
    }
  hexnum[len * 2] = 0;
}

#ifndef _LIBC
static char
hexval (char c)
{
  if (c >= '0' && c <= '9')
    return (c - '0');
  else if (c >= 'a' && c <= 'z')
    return (c - 'a' + 10);
  else if (c >= 'A' && c <= 'Z')
    return (c - 'A' + 10);
  else
    return -1;
}
#endif
