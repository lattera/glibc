/* Loop macro used in vector math functions tests.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This macro is used in VECTOR_WRAPPER macros for vector tests.  */
#define TEST_VEC_LOOP(len) 					\
  do								\
    {								\
      for (i = 1; i < len; i++)					\
        {							\
          if ((FLOAT) mr[0] != (FLOAT) mr[i])			\
            {							\
              return ((FLOAT) mr[0] + 0.1);			\
            }							\
        }							\
      return ((FLOAT) mr[0]);					\
    }								\
  while (0)

#define INIT_VEC_LOOP(vec, val, len)				\
  do								\
    {								\
      for (i = 0; i < len; i++)					\
        {							\
          vec[i] = val;						\
        }							\
    }								\
  while (0)
