#! /bin/sh
# Testing the implementation of the isxxx() and toxxx() functions.
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
tst_ctype=$1; shift
status=0

# Run the test programs.
rm -f ${common_objpfx}localedata/tst-ctype.out
for loc in C de_DE.ISO-8859-1 de_DE.UTF-8 en_US.ANSI_X3.4-1968 ja_JP.EUC-JP; do
  if test -f tst-ctype-$loc.in; then
    input=tst-ctype-$loc.in
  else
    input=/dev/null
  fi
  LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}iconvdata \
  LC_ALL=$loc ${tst_ctype} < $input \
    >> ${common_objpfx}localedata/tst-ctype.out || status=1
done

exit $status
