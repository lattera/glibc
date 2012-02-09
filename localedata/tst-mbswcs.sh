#! /bin/sh
# Testing the implementation of the mb*towc*() and wc*tomb*() functions.
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
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

common_objpfx=$1; shift
run_program_prefix=$1; shift

status=0

# Run the test programs.
LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}iconvdata \
${run_program_prefix} ${common_objpfx}localedata/tst-mbswcs1 \
  > ${common_objpfx}localedata/tst-mbswcs.out || status=1

LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}iconvdata \
${run_program_prefix} ${common_objpfx}localedata/tst-mbswcs2 \
  >> ${common_objpfx}localedata/tst-mbswcs.out || status=1

LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}iconvdata \
${run_program_prefix} ${common_objpfx}localedata/tst-mbswcs3 \
  >> ${common_objpfx}localedata/tst-mbswcs.out || status=1

LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}iconvdata \
${run_program_prefix} ${common_objpfx}localedata/tst-mbswcs4 \
  >> ${common_objpfx}localedata/tst-mbswcs.out || status=1

LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}iconvdata \
${run_program_prefix} ${common_objpfx}localedata/tst-mbswcs5 \
  >> ${common_objpfx}localedata/tst-mbswcs.out || status=1

exit $status
