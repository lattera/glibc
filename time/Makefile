# Copyright (C) 1991-2018 Free Software Foundation, Inc.
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
#	Makefile for time routines
#
subdir	:= time

include ../Makeconfig

headers := time.h sys/time.h sys/timeb.h bits/time.h			\
	   bits/types/clockid_t.h bits/types/clock_t.h			\
	   bits/types/struct_itimerspec.h				\
	   bits/types/struct_timespec.h bits/types/struct_timeval.h	\
	   bits/types/struct_tm.h bits/types/timer_t.h			\
	   bits/types/time_t.h

routines := offtime asctime clock ctime ctime_r difftime \
	    gmtime localtime mktime time		 \
	    gettimeofday settimeofday adjtime tzset	 \
	    tzfile getitimer setitimer			 \
	    stime dysize timegm ftime			 \
	    getdate strptime strptime_l			 \
	    strftime wcsftime strftime_l wcsftime_l	 \
	    timespec_get
aux :=	    era alt_digit lc-time-cleanup

tests	:= test_time clocktest tst-posixtz tst-strptime tst_wcsftime \
	   tst-getdate tst-mktime tst-mktime2 tst-ftime_l tst-strftime \
	   tst-mktime3 tst-strptime2 bug-asctime bug-asctime_r bug-mktime1 \
	   tst-strptime3 bug-getdate1 tst-strptime-whitespace tst-ftime \
	   tst-tzname tst-y2039

include ../Rules

ifeq ($(run-built-tests),yes)
LOCALES := de_DE.ISO-8859-1 en_US.ISO-8859-1 ja_JP.EUC-JP fr_FR.UTF-8 \
	   es_ES.UTF-8 pl_PL.UTF-8 ru_RU.UTF-8
include ../gen-locales.mk

$(objpfx)tst-ftime_l.out: $(gen-locales)
$(objpfx)tst-strptime.out: $(gen-locales)
endif

tz-cflags = -DTZDIR='"$(zonedir)"' \
	    -DTZDEFAULT='"$(localtime-file)"' \
	    -DTZDEFRULES='"$(posixrules-file)"'

CFLAGS-tzfile.c += $(tz-cflags)
CFLAGS-tzset.c += $(tz-cflags)
CFLAGS-getdate.c += -fexceptions

# Don't warn about Y2k problem in strftime format string.
CFLAGS-test_time.c += -Wno-format

tst-getdate-ENV= DATEMSK=datemsk TZDIR=${common-objpfx}timezone/testdata
test_time-ARGS= EST5EDT CST

tst-tzname-ENV = TZDIR=${common-objpfx}timezone/testdata
CPPFLAGS-tst-tzname.c += -DTZDEFRULES='"$(posixrules-file)"'

bug-getdate1-ARGS = ${objpfx}bug-getdate1-fmt
