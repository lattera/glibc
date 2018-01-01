/* Define struct __cancel_jmp_buf_tag.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

#ifndef ____cancel_jmp_buf_tag_defined
#define ____cancel_jmp_buf_tag_defined 1

#include <bits/types/__sigset_t.h>

struct __cancel_jmp_buf_tag
  {
    __jmp_buf __cancel_jmp_buf;
    int __mask_was_saved;
    __sigset_t __saved_mask;
  };

#endif
