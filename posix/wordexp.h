/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_WORDEXP_H

#define	_WORDEXP_H	1
#include <features.h>

__BEGIN_DECLS

/* Bits set in the FLAGS argument to `wordexp'.  */
enum
  {
    WRDE_DOOFFS = (1 << 0),	/* Insert PWORDEXP->we_offs NULLs.  */
    WRDE_APPEND = (1 << 1),	/* Append to results of a previous call.  */
    WRDE_NOCMD = (1 << 2),	/* Don't do command substitution.  */
    WRDE_REUSE = (1 << 3),	/* Reuse storage in PWORDEXP.  */
    WRDE_SHOWERR = (1 << 4),	/* Don't redirect stderr to /dev/null.  */
    WRDE_UNDEF = (1 << 5),	/* Error for expanding undefined variables.  */
    __WRDE_FLAGS = (WRDE_DOOFFS | WRDE_APPEND | WRDE_NOCMD |
		    WRDE_REUSE | WRDE_SHOWERR | WRDE_UNDEF),
  };

/* Structure describing a word-expansion run.  */
typedef struct
  {
    int we_wordc;		/* Count of words matched.  */
    char **we_wordv;		/* List of expanded words.  */
    int we_offs;		/* Slots to reserve in `we_wordv'.  */
  } wordexp_t;

/* Possible nonzero return values from `wordexp'.  */
enum
  {
    WRDE_NOSPACE = 1,		/* Ran out of memory.  */
    WRDE_BADCHAR,		/* A metachar appears in the wrong place.  */
    WRDE_BADVAL,		/* Undefined var reference with WRDE_UNDEF.  */
    WRDE_CMDSUB,		/* Command substitution with WRDE_NOCMD.  */
    WRDE_SYNTAX			/* Shell syntax error.  */
  };

/* Do word expansion of WORDS into PWORDEXP.  */
extern int wordexp __P ((__const char *__words, wordexp_t * __pwordexp,
			 int __flags));

/* Free the storage allocated by a `wordexp' call.  */
extern void wordfree __P ((wordexp_t *));

__END_DECLS

#endif /* wordexp.h  */
