#! /bin/sh
# Testing the implementation of the isxxx() and toxxx() functions.
# Copyright (C) 2000 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

common_objpfx=$1; shift

generate_locale ()
{
    charmap=$1
    input=$2
    out=$3
    I18NPATH=. GCONV_PATH=${common_objpfx}iconvdata \
    ${common_objpfx}elf/ld.so --library-path $common_objpfx \
    ${common_objpfx}locale/localedef --quiet -c -f $charmap -i $input \
      ${common_objpfx}localedata/$out

    if [ $? -ne 0 ]; then
	echo "Charmap: \"${charmap}\" Inputfile: \"${input}\"" \
	     "Outputdir: \"${out}\" failed"
	exit 1
    fi
}

generate_locale EUC-JP ja_JP ja_JP.EUC-JP

status=0

# Run the test programs.
rm -f ${common_objpfx}localedata/tst-ctype.out
for loc in de_DE en_US ja_JP.EUC-JP; do
  if test -f tst-ctype-$loc.in; then
    input=tst-ctype-$loc.in
  else
    input=/dev/null
  fi
  LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}iconvdata \
  LC_ALL=$loc ${common_objpfx}elf/ld.so --library-path $common_objpfx \
    ${common_objpfx}localedata/tst-ctype < $input \
    >> ${common_objpfx}localedata/tst-ctype.out || status=1
done

exit $status
