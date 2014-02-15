#! /bin/sh
# Output a test status line.
# Copyright (C) 2012-2014 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

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

# usage: evaluate-test.sh test_name rc

test_name=$1
rc=$2

if [ $rc -eq 0 ]; then
  result="PASS"
else
  result="FAIL"
fi

echo "$result: $test_name"
echo "original exit status $rc"
exit $rc
