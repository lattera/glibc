#! /bin/sh
# Generate test locale files.
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

common_objpfx="$1"; shift
localedef="$1"; shift
locfile="$1"; shift

generate_locale ()
{
  charmap=$1
  input=$2
  out=$3
  if I18NPATH=. GCONV_PATH=${common_objpfx}iconvdata \
     ${localedef} --quiet -c -f $charmap -i $input \
		  ${common_objpfx}localedata/$out
  then
    # The makefile checks the timestamp of the LC_CTYPE file,
    # but localedef won't have touched it if it was able to
    # hard-link it to an existing file.
    touch ${common_objpfx}localedata/$out/LC_CTYPE
  else
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
