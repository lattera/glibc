#! /bin/sh
# Test of gettext functions.
# Copyright (C) 2000-2012 Free Software Foundation, Inc.
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

set -e

common_objpfx=$1
run_program_prefix=$2
objpfx=$3
malloc_trace=$4

LC_ALL=C
export LC_ALL

# Generate the test data.

# Create the locale directories.
mkdir -p ${objpfx}localedir/existing-locale/LC_MESSAGES
for f in ADDRESS COLLATE CTYPE IDENTIFICATION MEASUREMENT MONETARY NAME NUMERIC PAPER TELEPHONE TIME; do
  cp -f ${common_objpfx}localedata/de_DE.UTF-8/LC_$f \
        ${objpfx}localedir/existing-locale
done
cp -f ${common_objpfx}localedata/de_DE.UTF-8/LC_MESSAGES/SYS_LC_MESSAGES \
      ${objpfx}localedir/existing-locale/LC_MESSAGES

# Create the domain directories.
mkdir -p ${objpfx}domaindir/existing-locale/LC_MESSAGES
mkdir -p ${objpfx}domaindir/existing-locale/LC_TIME
# Populate them.
msgfmt -o ${objpfx}domaindir/existing-locale/LC_MESSAGES/existing-domain.mo \
       -f ../po/de.po
msgfmt -o ${objpfx}domaindir/existing-locale/LC_TIME/existing-time-domain.mo \
       -f ../po/de.po

GCONV_PATH=${common_objpfx}iconvdata
export GCONV_PATH
LOCPATH=${common_objpfx}localedata
export LOCPATH

# Now run the test.
MALLOC_TRACE=$malloc_trace LOCPATH=${objpfx}localedir:$LOCPATH \
${run_program_prefix} \
${objpfx}tst-gettext > ${objpfx}tst-gettext.out ${objpfx}domaindir

exit $?
