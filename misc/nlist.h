/* Copyright (C) 1991, 1992, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef	_NLIST_H
#define	_NLIST_H	1

#include <features.h>

__BEGIN_DECLS

/* Structure describing a symbol-table entry.  */
struct nlist
{
  char *n_name;
  unsigned char n_type;
  char n_other;
  short int n_desc;
  unsigned long int n_value;
};

#define	N_NLIST_DECLARED
#include <a.out.h>


/* Search the executable FILE for symbols matching those in NL,
   which is terminated by an element with a NULL `n_un.n_name' member,
   and fill in the elements of NL.  */
extern int nlist __P ((__const char *__file, struct nlist * __nl));


__END_DECLS

#endif /* nlist.h  */
