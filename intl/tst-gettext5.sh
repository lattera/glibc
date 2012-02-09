#! /bin/sh
# Test that gettext() in multithreaded applications works correctly if
# different threads operate in different locales referring to the same
# catalog file but with different encodings.
# Copyright (C) 2001, 2002, 2005 Free Software Foundation, Inc.
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

LC_ALL=C
export LC_ALL

# Create the domain directories.
mkdir -p ${objpfx}domaindir/de_DE/LC_MESSAGES
# Populate them.
msgfmt -o ${objpfx}domaindir/de_DE/LC_MESSAGES/codeset.mo tstcodeset.po

GCONV_PATH=${common_objpfx}iconvdata
export GCONV_PATH
LOCPATH=${common_objpfx}localedata
export LOCPATH

${run_program_prefix} ${objpfx}tst-gettext5 > ${objpfx}tst-gettext5.out

exit $?
