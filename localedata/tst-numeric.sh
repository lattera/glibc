#! /bin/sh
# Testing the implementation of LC_NUMERIC and snprintf(3).
# Copyright (C) 1996, 1997, 1998, 2000, 2003 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Jochen Hein <jochen.hein@delphi.central.de>, 1997.
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
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

common_objpfx=$1
run_program_prefix=$2
datafile=$3

# Run the tests.
errcode=0
# There's a TAB for IFS
while IFS="	" read locale format value expect; do
    case "$locale" in '#'*) continue ;; esac
    if [ -n "$format" ]; then
	if LOCPATH=${common_objpfx}localedata \
	    GCONV_PATH=${common_objpfx}/iconvdata \
	    ${run_program_prefix} ${common_objpfx}localedata/tst-numeric \
	    "$locale" "$format" "$value" "$expect"
	then
	    echo "Locale: \"${locale}\" Format: \"${format}\"" \
		 "Value: \"${value}\" Expect: \"${expect}\"  passed"
	else
	    errcode=$?
	    echo "Locale: \"${locale}\" Format: \"${format}\"" \
		 "Value: \"${value}\" Expect: \"${expect}\"    failed"
	fi
    fi
done < $datafile

exit $errcode
# Local Variables:
#  mode:shell-script
# End:
