#! /bin/bash
# Copyright (C) 2005,2007,2009,2011 Free Software Foundation, Inc.
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
#
includedir="$1"
objpfx="$2"

# To avoid long paths.
cd "$objpfx"

# Search all dependency files for file names in the include directory.
# There are a few system headers we are known to use.
exec ${AWK} -v includedir="$includedir" '
BEGIN {
  status = 0
  exclude = "^" includedir \
    "/(asm[-/]|linux/|selinux/|gd|nss3/|sys/capability\\.h|libaudit\\.h)"
}
/^[^ ]/ && $1 ~ /.*:/ { obj = $1 }
{
  for (i = 1; i <= NF; ++i) {
    if ($i ~ ("^" includedir) && $i !~ exclude) {
      print "***", obj, "uses", $i
      status = 1
    }
  }
}
END { exit status }' */*.{o,os,oS}.d
