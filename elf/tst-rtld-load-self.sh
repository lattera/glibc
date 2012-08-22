#! /bin/sh
# Test how rtld loads itself.
# Copyright (C) 2012 Free Software Foundation, Inc.
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

set -e

rtld=$1
result=0

echo '# normal mode'
$rtld $rtld 2>&1 && rc=0 || rc=$?
echo "# exit status $rc"
test $rc -le 127 || result=1

echo '# list mode'
$rtld --list $rtld 2>&1 && rc=0 || rc=$?
echo "# exit status $rc"
test $rc -eq 0 || result=1

echo '# verify mode'
$rtld --verify $rtld 2>&1 && rc=0 || rc=$?
echo "# exit status $rc"
test $rc -eq 2 || result=1

echo '# trace mode'
LD_TRACE_LOADED_OBJECTS=1 $rtld $rtld 2>&1 && rc=0 || rc=$?
echo "# exit status $rc"
test $rc -eq 0 || result=1

exit $result
