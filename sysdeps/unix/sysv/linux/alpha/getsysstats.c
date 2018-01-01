/* Determine various system internal values, Linux/Alpha version.
   Copyright (C) 1999-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@suse.de>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */


/* We need to define a special parser for /proc/cpuinfo.  */
#define GET_NPROCS_PARSER(FD, BUFFER, CP, RE, BUFFER_END, RESULT)	   \
  do									   \
    {									   \
      /* Find the line that contains the information about the number of   \
	 active cpus.  We don't have to fear extremely long lines since	   \
	 the kernel will not generate them.  8192 bytes are really enough. \
	 If there is no "CPUs ..." line then we are on a UP system.  */	   \
      char *l;								   \
      (RESULT) = 1;							   \
      while ((l = next_line (FD, BUFFER, &CP, &RE, BUFFER_END)) != NULL)  \
	if ((sscanf (BUFFER, "cpus active : %d", &(RESULT)) == 1)	   \
	    || (sscanf (BUFFER, "CPUs probed %*d active %d",		   \
			&(RESULT)) == 1))  				   \
	  break;							   \
    }									   \
  while (0)


/* On the Alpha we can distinguish between the number of configured and
   active cpus.  */
#define GET_NPROCS_CONF_PARSER(FP, BUFFER, RESULT)			   \
  do									   \
    {									   \
      /* Find the line that contains the information about the number of   \
	 probed cpus.  We don't have to fear extremely long lines since	   \
	 the kernel will not generate them.  8192 bytes are really enough. \
	 If there is no "CPUs ..." line then we are on a UP system.  */	   \
      (RESULT) = 1;							   \
      while (__fgets_unlocked ((BUFFER), sizeof (BUFFER), (FP)) != NULL)   \
	if ((sscanf (buffer, "cpus detected : %d", &(RESULT)) == 1)	   \
	    || (sscanf (buffer, "CPUs probed %d", &(RESULT)) == 1))	   \
	  break;							   \
    }									   \
  while (0)

#include <sysdeps/unix/sysv/linux/getsysstats.c>
