/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <a.out.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Search the executable FILE for symbols matching those in NL,
   which is terminated by an element with a NULL `n_un.n_name' member,
   and fill in the elements of NL.  */
int
DEFUN(nlist, (file, nl),
      CONST char *file AND struct nlist *nl)
{
  FILE *f;
  struct exec header;
  size_t nsymbols;
  struct nlist *symbols;
  unsigned long int string_table_size;
  char *string_table;
  register size_t i;

  if (nl == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  f = fopen(file, "r");
  if (f == NULL)
    return -1;

  if (fread((PTR) &header, sizeof(header), 1, f) != 1)
    goto lose;

  if (fseek(f, N_SYMOFF(header), SEEK_SET) != 0)
    goto lose;

  symbols = (struct nlist *) __alloca(header.a_syms);
  nsymbols = header.a_syms / sizeof(symbols[0]);

  if (fread((PTR) symbols, sizeof(symbols[0]), nsymbols, f) != nsymbols)
    goto lose;

  if (fread((PTR) &string_table_size, sizeof(string_table_size), 1, f) != 1)
    goto lose;
  string_table_size -= sizeof(string_table_size);

  string_table = (char *) __alloca(string_table_size);
  if (fread((PTR) string_table, string_table_size, 1, f) != 1)
    goto lose;

  for (i = 0; i < nsymbols; ++i)
    {
      register struct nlist *nlp;
      for (nlp = nl; nlp->n_un.n_name != NULL; ++nlp)
	if (!strcmp(nlp->n_un.n_name,
		    &string_table[symbols[i].n_un.n_strx -
				  sizeof(string_table_size)]))
	  {
	    char *CONST name = nlp->n_un.n_name;
	    *nlp = symbols[i];
	    nlp->n_un.n_name = name;
	  }
    }

  (void) fclose(f);
  return 0;

 lose:;
  (void) fclose(f);
  return -1;
}
