#! /bin/bash
# Copyright (C) 2005, 2007, 2009 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.
#
includedir="$1"
objpfx="$2"

# To avoid long paths.
cd "$objpfx"


# Search all dependency files for file names in the include directory.
# There are a few system headers we are known to use.
if fgrep "$includedir" */*.{o,os,oS}.d |
fgrep -v "$includedir/asm" |
fgrep -v "$includedir/linux" |
fgrep -v "$includedir/selinux" |
fgrep -v "$includedir/sys/capability.h" |
fgrep -v "$includedir/gd" |
fgrep -v "$includedir/nss3"; then
  # If we found a match something is wrong.
  exit 1
fi

exit 0
