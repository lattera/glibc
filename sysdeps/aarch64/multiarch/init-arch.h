/* Define INIT_ARCH so that midr is initialized before use by IFUNCs.
   This file is part of the GNU C Library.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.

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

#include <ldsodefs.h>

#define INIT_ARCH()							      \
  uint64_t __attribute__((unused)) midr =				      \
    GLRO(dl_aarch64_cpu_features).midr_el1;				      \
  unsigned __attribute__((unused)) zva_size =				      \
    GLRO(dl_aarch64_cpu_features).zva_size;
