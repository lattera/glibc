/* Determine various system internal values, Linux/Sparc version.
   Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@suse.de> and
		  Jakub Jelinek <jj@ultra.linux.cz>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */


/* We need to define a special parser for /proc/cpuinfo.  */
#define GET_NPROCS_PARSER(FP, BUFFER, RESULT)				  \
  do									  \
    {									  \
      (RESULT) = 0;							  \
      /* Find the line that contains the information about the number of  \
	 active cpus.  We don't have to fear extremely long lines since	  \
	 the kernel will not generate them.  8192 bytes are really	  \
	 enough.  */							  \
      while (fgets_unlocked (BUFFER, sizeof (BUFFER), FP) != NULL)	  \
	if (sscanf (BUFFER, "ncpus active : %d", &(RESULT)) == 1)	  \
	  break;							  \
    }									  \
  while (0)


/* On the Sparc we can distinguish between the number of configured and
   active cpus.  */
#define GET_NPROCS_CONF_PARSER(FP, BUFFER, RESULT)			 \
  do									 \
    {									 \
      (RESULT) = 0;							 \
      /* Find the line that contains the information about the number of \
	 probed cpus.  We don't have to fear extremely long lines since	 \
	 the kernel will not generate them.  8192 bytes are really	 \
	 enough.  */							 \
      while (fgets_unlocked ((BUFFER), sizeof (BUFFER), (FP)) != NULL)	 \
	if (sscanf (buffer, "ncpus probed : %d", &(RESULT)) == 1)	 \
	  break;							 \
    }									 \
  while (0)

#include <sysdeps/unix/sysv/linux/getsysstats.c>
