#!/bin/sh
# Script to validate NaCl binaries after linking.

# Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

# See sysdeps/nacl/Makefile for how this script is invoked.
READELF="$1"
binary="$2"

if [ -z "$NACL_SDK_ROOT" ]; then
  echo >&2 "$0: NACL_SDK_ROOT must be set in the environment"
  exit 77
fi

ncval="${NACL_SDK_ROOT}/tools/ncval"

if [ ! -x "$ncval" ]; then
  echo >&2 "$0: No ncval binary in $ncval"
  exit 77
fi

"${READELF}" -Wl "$binary" | awk '
BEGIN { saw_load = saw_text = 0 }
$1 == "LOAD" {
  saw_load = 1;
  if (/ R.E /) saw_code = 1;
}
END {
  exit (saw_code ? 11 : saw_load ? 22 : 1);
}
'
case $? in
11)
  # We saw a code segment, so we can try ncval.
  ;;
22)
  # We saw LOAD segments but none of them were code.
  echo >&2 "+++ No code: $binary"
  exit 0
  ;;
*)
  # Something funny going on.
  echo >&2 "*** Failed to analyze: $binary"
  exit 2
  ;;
esac

if "$ncval" "$binary"; then
  echo >&2 "+++ Validated: $binary"
  exit 0
else
  echo >&2 "*** Validation failed: $binary"
  exit 2
fi
