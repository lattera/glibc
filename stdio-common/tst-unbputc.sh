#! /bin/sh
# Testing the stdio implementation
# Copyright (C) 2000 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
#

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

common_objpfx=$1; shift
run_program_prefix=$1; shift

status=0

${run_program_prefix} \
  ${common_objpfx}stdio-common/tst-unbputc \
    2> ${common_objpfx}stdio-common/tst-unbputc.out || status=1

(echo -n 12 | cmp ${common_objpfx}stdio-common/tst-unbputc.out -) || status=1

exit $status
