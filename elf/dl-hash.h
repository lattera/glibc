/* Compute hash alue for given string according to ELF standard.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#ifndef _DL_HASH_H
#define _DL_HASH_H	1


/* This is the hashing function specified by the ELF ABI.  In the
   first five operations now overflow is possible so we optimized it a
   bit.  */
static inline unsigned
_dl_elf_hash (const char *name)
{
  unsigned long int hash = 0;
  if (*name != '\0')
    {
      hash = (hash << 4) + *name++;
      if (*name != '\0')
	{
	  hash = (hash << 4) + *name++;
	  if (*name != '\0')
	    {
	      hash = (hash << 4) + *name++;
	      if (*name != '\0')
		{
		  hash = (hash << 4) + *name++;
		  if (*name != '\0')
		    {
		      hash = (hash << 4) + *name++;
		      while (*name != '\0')
			{
			  unsigned long int hi;
			  hash = (hash << 4) + *name++;
			  hi = hash & 0xf0000000;
			  if (hi != 0)
			    {
			      hash ^= hi >> 24;
			      /* The ELF ABI says `hash &= ~hi', but
				 this is equivalent in this case and
				 on some machines one insn instead of
				 two.  */
			      hash ^= hi;
			    }
			}
		    }
		}
	    }
	}
    }
  return hash;
}

#endif /* dl-hash.h */
