#! /bin/sh
# Test that the gettext() results come out in the correct encoding for
# locales that differ only in their encoding.
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
objpfx=$2

LC_ALL=C
export LC_ALL

# Generate the test data.
msgfmt -o ${objpfx}codeset.mo.$$ tstcodeset.po || exit
# Create the domain directories.
mkdir -p ${objpfx}domaindir/de_DE/LC_MESSAGES
# Populate them.
mv -f ${objpfx}codeset.mo.$$ ${objpfx}domaindir/de_DE/LC_MESSAGES/codeset.mo

GCONV_PATH=${common_objpfx}iconvdata
export GCONV_PATH
LOCPATH=${common_objpfx}localedata
export LOCPATH

${common_objpfx}elf/ld.so --library-path $common_objpfx \
${objpfx}tst-gettext3 > ${objpfx}tst-gettext3.out

exit $?
