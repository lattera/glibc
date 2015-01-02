/* Copyright (C) 1997-2015 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* We need to define a special parser for /proc/cpuinfo.  */
#define GET_NPROCS_PARSER(FD, BUFFER, CP, RE, BUFFER_END, RESULT)              \
  do                                                                           \
    {                                                                          \
      (RESULT) = 0;                                                            \
      /* Read all lines and count the lines starting with the string           \
         "CPU-Family:".  We don't have to fear extremely long lines since      \
         the kernel will not generate them.  8192 bytes are really enough.  */ \
      char *l;                                                                 \
      while ((l = next_line (FD, BUFFER, &CP, &RE, BUFFER_END)) != NULL)       \
      if (strncmp (l, "CPU-Family:", 11) == 0)                                 \
        ++(RESULT);                                                            \
    }                                                                          \
  while (0)

#include <sysdeps/unix/sysv/linux/getsysstats.c>
