# Copyright (C) 1991-1999, 2000, 2001 Free Software Foundation, Inc.
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
#	Makefile for time routines
#
subdir	:= time

headers	:= time.h sys/time.h sys/timeb.h bits/time.h

routines := offtime asctime clock ctime ctime_r difftime \
	    gmtime localtime mktime strftime time	 \
	    gettimeofday settimeofday adjtime tzset	 \
	    tzfile getitimer setitimer			 \
	    stime dysize timegm ftime			 \
	    strptime getdate wcsftime

distribute := datemsk

tests	:= test_time clocktest tst-posixtz tst-strptime tst_wcsftime \
	   tst-getdate tst-mktime

include ../Rules

tz-cflags = -DTZDIR='"$(zonedir)"' \
	    -DTZDEFAULT='"$(localtime-file)"' \
	    -DTZDEFRULES='"$(posixrules-file)"'

CFLAGS-tzfile.c = $(tz-cflags)
CFLAGS-tzset.c = $(tz-cflags)

# Don't warn about Y2k problem in strftime format string.
CFLAGS-test_time.c = -Wno-format

tst-getdate-ENV= DATEMSK=datemsk
test_time-ARGS= EST5EDT CST
