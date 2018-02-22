/* A Thunderx2 Optimized memcpy implementation for AARCH64.
   Copyright (C) 2018 Free Software Foundation, Inc.

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

/* The actual code in this memcpy and memmove is in memcpy_thunderx.S.
   The only real differences are with the prefetching instructions.  */

#define MEMCPY __memcpy_thunderx2
#define MEMMOVE __memmove_thunderx2
#define USE_THUNDERX2

#include "memcpy_thunderx.S"
