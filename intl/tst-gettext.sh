#! /bin/sh
# Test of gettext functions.
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

common_objpfx=$1
objpfx=$2
malloc_trace=$3

GCONV_PATH=${common_objpfx}iconvdata
export GCONV_PATH
LOCPATH=${common_objpfx}localedata
export LOCPATH
LC_ALL=C
export LC_ALL

# Generate the test data.
test -d ${objpfx}domaindir || mkdir ${objpfx}domaindir
test -d ${objpfx}localedir || mkdir ${objpfx}localedir
# Create the domain directories.
test -d ${objpfx}domaindir/existing-locale || mkdir ${objpfx}domaindir/existing-locale
test -d ${objpfx}domaindir/existing-locale/LC_MESSAGES || mkdir ${objpfx}domaindir/existing-locale/LC_MESSAGES
test -d ${objpfx}domaindir/existing-locale/LC_TIME || mkdir ${objpfx}domaindir/existing-locale/LC_TIME
# Create the locale directories.
test -d ${objpfx}localedir/existing-locale || {
  mkdir ${objpfx}localedir/existing-locale
  for f in ADDRESS COLLATE CTYPE IDENTIFICATION MEASUREMENT MONETARY NAME NUMERIC PAPER TELEPHONE TIME; do
    cp ${common_objpfx}localedata/de_DE.ISO-8859-1/LC_$f \
       ${objpfx}localedir/existing-locale
  done
}
test -d ${objpfx}localedir/existing-locale/LC_MESSAGES || {
  mkdir ${objpfx}localedir/existing-locale/LC_MESSAGES
  cp ${common_objpfx}localedata/de_DE.ISO-8859-1/LC_MESSAGES/SYS_LC_MESSAGES \
     ${objpfx}localedir/existing-locale/LC_MESSAGES
}

# Populate them.
msgfmt -o ${objpfx}domaindir/existing-locale/LC_MESSAGES/existing-domain.mo \
       ../po/de.po
msgfmt -o ${objpfx}domaindir/existing-locale/LC_TIME/existing-time-domain.mo \
       ../po/de.po

# Now run the test.
MALLOC_TRACE=$malloc_trace LOCPATH=${objpfx}localedir:$LOCPATH \
${common_objpfx}elf/ld.so --library-path $common_objpfx \
${objpfx}tst-gettext > ${objpfx}tst-gettext.out ${objpfx}domaindir

exit $?
