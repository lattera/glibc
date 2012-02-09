#! /bin/sh
# Test of freopen.
# Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

common_objpfx=$1
run_program_prefix=$2
objpfx=$3

${run_program_prefix} ${objpfx}test-freopen ${objpfx}test-freopen.out &&
cmp ${objpfx}test-freopen.out - <<"EOF"
Hello world (mb)
Hello world (wc)
EOF

exit $?
