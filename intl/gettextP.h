/* Header describing internals of gettext library
   Copyright (C) 1995-1999, 2000 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@cygnus.com>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _GETTEXTP_H
#define _GETTEXTP_H

#ifdef _LIBC
# include "../iconv/gconv_int.h"
#else
# if HAVE_ICONV
#  include <iconv.h>
# endif
#endif

#include "loadinfo.h"

/* @@ end of prolog @@ */

#ifndef PARAMS
# if __STDC__
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif

#ifndef internal_function
# define internal_function
#endif

/* Tell the compiler when a conditional or integer expression is
   almost always true or almost always false.  */
#ifndef HAVE_BUILTIN_EXPECT
# define __builtin_expect(expr, val) (expr)
#endif

#ifndef W
# define W(flag, data) ((flag) ? SWAP (data) : (data))
#endif


#ifdef _LIBC
# include <byteswap.h>
# define SWAP(i) bswap_32 (i)
#else
static nls_uint32 SWAP PARAMS ((nls_uint32 i));

static inline nls_uint32
SWAP (i)
     nls_uint32 i;
{
  return (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | (i >> 24);
}
#endif


/* This is the representation of the expressions to determine the
   plural form.  */
struct expression
{
  enum operator
  {
    var,			/* The variable "n".  */
    num,			/* Decimal number.  */
    mult,			/* Multiplication.  */
    divide,			/* Division.  */
    module,			/* Module operation.  */
    plus,			/* Addition.  */
    minus,			/* Subtraction.  */
    equal,			/* Comparision for equality.  */
    not_equal,			/* Comparision for inequality.  */
    land,			/* Logical AND.  */
    lor,			/* Logical OR.  */
    qmop			/* Question mark operator.  */
  } operation;
  union
  {
    unsigned long int num;	/* Number value for `num'.  */
    struct
    {
      struct expression *left;	/* Left expression in binary operation.  */
      struct expression *right;	/* Right expression in binary operation.  */
    } args2;
    struct
    {
      struct expression *bexp;	/* Boolean expression in ?: operation.  */
      struct expression *tbranch; /* True-branch in ?: operation.  */
      struct expression *fbranch; /* False-branch in ?: operation.  */
    } args3;
  } val;
};

/* This is the data structure to pass information to the parser and get
   the result in a thread-safe way.  */
struct parse_args
{
  const char *cp;
  struct expression *res;
};


struct loaded_domain
{
  const char *data;
  int use_mmap;
  size_t mmap_size;
  int must_swap;
  nls_uint32 nstrings;
  struct string_desc *orig_tab;
  struct string_desc *trans_tab;
  nls_uint32 hash_size;
  nls_uint32 *hash_tab;
#ifdef _LIBC
  __gconv_t conv;
#else
# if HAVE_ICONV
  iconv_t conv;
# endif
#endif
  char **conv_tab;

  struct expression *plural;
  unsigned long int nplurals;
};

struct binding
{
  struct binding *next;
  char *dirname;
  char *codeset;
#ifdef __GNUC__
  char domainname[0];
#else
  char domainname[1];
#endif
};

extern int _nl_msg_cat_cntr;

struct loaded_l10nfile *_nl_find_domain PARAMS ((const char *__dirname,
						 char *__locale,
						 const char *__domainname,
					      struct binding *__domainbinding))
     internal_function;
void _nl_load_domain PARAMS ((struct loaded_l10nfile *__domain))
     internal_function;
void _nl_unload_domain PARAMS ((struct loaded_domain *__domain))
     internal_function;

#ifdef _LIBC
extern char *__ngettext PARAMS ((const char *msgid1, const char *msgid2,
				 unsigned long int n));
extern char *__dngettext PARAMS ((const char *domainname, const char *msgid1,
				  const char *msgid2, unsigned long int n));
extern char *__dcngettext PARAMS ((const char *domainname, const char *msgid1,
				   const char *msgid2, unsigned long int n,
				   int category));
extern char *__dcigettext PARAMS ((const char *domainname, const char *msgid1,
				   const char *msgid2, int plural,
				   unsigned long int n, int category));
#else
extern char *ngettext__ PARAMS ((const char *msgid1, const char *msgid2,
				 unsigned long int n));
extern char *dngettext__ PARAMS ((const char *domainname, const char *msgid1,
				  const char *msgid2, unsigned long int n));
extern char *dcngettext__ PARAMS ((const char *domainname, const char *msgid1,
				   const char *msgid2, unsigned long int n,
				   int category));
extern char *dcigettext__ PARAMS ((const char *domainname, const char *msgid1,
				   const char *msgid2, int plural,
				   unsigned long int n, int category));
#endif

extern int __gettextdebug;
extern void __gettext_free_exp (struct expression *exp) internal_function;
extern int __gettextparse (void *arg);

/* @@ begin of epilog @@ */

#endif /* gettextP.h  */
