#! /bin/sh
# Generate test locale files.
# Copyright (C) 2000-2001 Free Software Foundation, Inc.
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

common_objpfx="$1"; shift
localedef="$1"; shift
locfile="$1"; shift

generate_locale ()
{
    charmap=$1
    input=$2
    out=$3
    I18NPATH=. GCONV_PATH=${common_objpfx}iconvdata \
    ${localedef} --quiet -c -f $charmap -i $input \
      ${common_objpfx}localedata/$out

    if [ $? -ne 0 ]; then
	echo "Charmap: \"${charmap}\" Inputfile: \"${input}\"" \
	     "Outputdir: \"${out}\" failed"
	exit 1
    fi
}

locfile=`echo $locfile|sed 's|.*/\([^/]*/LC_CTYPE\)|\1|'`
locale=`echo $locfile|sed 's|\([^.]*\)[.].*/LC_CTYPE|\1|'`
charmap=`echo $locfile|sed 's|[^.]*[.]\(.*\)/LC_CTYPE|\1|'`

echo "Generating locale $locale.$charmap: this might take a while..."
generate_locale `echo $charmap | sed -e s/SJIS/SHIFT_JIS/` $locale \
		$locale.$charmap
