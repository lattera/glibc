/* Define one NaCl interface table.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include "nacl-interfaces.h"

#define PASTE(a, b)     PASTE_1 (a, b)
#define PASTE_1(a, b)   a##b
#define STRINGIFY(x)    STRINGIFY_1 (x)
#define STRINGIFY_1(x)  #x

#if IS_IN (rtld) && PASTE (MODULE_, INTERFACE_MODULE) != MODULE_rtld
# error "This interface is also needed in rtld."
#endif

#define SECTION(which) \
  section ("nacl_"  STRINGIFY (INTERFACE_CATEGORY) "_interface_" #which)

static const struct nacl_interface PASTE (desc_, INTERFACE_TYPE)
  __attribute__ ((used, SECTION (names))) =
{
  .table_size = sizeof (struct INTERFACE_TYPE),
  .namelen = sizeof INTERFACE_STRING,
  .name = INTERFACE_STRING
};

struct INTERFACE_TYPE PASTE (__, INTERFACE_TYPE)
  __attribute__ ((SECTION (tables)));
PASTE (INTERFACE_MODULE, _hidden_data_def) (PASTE (__, INTERFACE_TYPE))
