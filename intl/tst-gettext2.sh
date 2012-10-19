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

LC_ALL=C
export LC_ALL

# Generate the test data.
mkdir -p ${objpfx}domaindir
# Create the locale directories.
test -d ${objpfx}domaindir/lang1 || {
  mkdir ${objpfx}domaindir/lang1
  for f in ADDRESS COLLATE CTYPE IDENTIFICATION MEASUREMENT MONETARY NAME NUMERIC PAPER TELEPHONE TIME; do
    cp ${common_objpfx}localedata/de_DE.ISO-8859-1/LC_$f \
       ${objpfx}domaindir/lang1
  done
}
test -d ${objpfx}domaindir/lang2 || {
  mkdir ${objpfx}domaindir/lang2
  for f in ADDRESS COLLATE CTYPE IDENTIFICATION MEASUREMENT MONETARY NAME NUMERIC PAPER TELEPHONE TIME; do
    cp ${common_objpfx}localedata/de_DE.ISO-8859-1/LC_$f \
       ${objpfx}domaindir/lang2
  done
}
test -d ${objpfx}domaindir/lang1/LC_MESSAGES || {
  mkdir ${objpfx}domaindir/lang1/LC_MESSAGES
  cp ${common_objpfx}localedata/de_DE.ISO-8859-1/LC_MESSAGES/SYS_LC_MESSAGES \
     ${objpfx}domaindir/lang1/LC_MESSAGES
}
test -d ${objpfx}domaindir/lang2/LC_MESSAGES || {
  mkdir ${objpfx}domaindir/lang2/LC_MESSAGES
  cp ${common_objpfx}localedata/de_DE.ISO-8859-1/LC_MESSAGES/SYS_LC_MESSAGES \
     ${objpfx}domaindir/lang2/LC_MESSAGES
}

# Populate them.
msgfmt -o ${objpfx}domaindir/lang1/LC_MESSAGES/tstlang.mo \
       tstlang1.po

msgfmt -o ${objpfx}domaindir/lang2/LC_MESSAGES/tstlang.mo \
       tstlang2.po

GCONV_PATH=${common_objpfx}iconvdata
export GCONV_PATH
LOCPATH=${objpfx}domaindir
export LOCPATH

# Now run the test.
${run_program_prefix} \
${objpfx}tst-gettext2 > ${objpfx}tst-gettext2.out ${objpfx}domaindir &&
cmp ${objpfx}tst-gettext2.out - <<EOF
String1 - Lang1: 1st string
String2 - Lang1: 2nd string
String1 - Lang2: 1st string
String2 - Lang2: 2nd string
String1 - First string for testing.
String2 - Another string for testing.
EOF

exit $?
