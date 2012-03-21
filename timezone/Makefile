# Copyright (C) 1998-2000,2002,2005,2007,2012 Free Software Foundation, Inc.
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
#	Makefile for timezone information
#
subdir	:= timezone

extra-objs := scheck.o ialloc.o

others	:= zdump zic
tests	:= test-tz tst-timezone

# pacificnew doesn't compile; if it is to be used, it should be included in
# northamerica.
tzbases := africa antarctica asia australasia europe northamerica \
	   southamerica etcetera factory \
	   solar87 solar88 solar89
tzlinks := backward systemv
tzfiles := $(tzbases) $(tzlinks)

generated := $(addprefix z.,$(tzfiles))
install-sbin := zic zdump

generated-dirs = testdata

all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig	# Get objpfx defined so we can use it below.

CPPFLAGS-zic = -DNOT_IN_libc

ifeq ($(have-ksh),yes)
install-bin-script = tzselect
generated += tzselect
endif

include ../Rules


$(objpfx)zic: $(objpfx)scheck.o $(objpfx)ialloc.o

tz-cflags = -DTZDIR='"$(zonedir)"' \
	    -DTZDEFAULT='"$(localtime-file)"' \
	    -DTZDEFRULES='"$(posixrules-file)"' \
	    -DTM_GMTOFF=tm_gmtoff -DTM_ZONE=tm_zone

CFLAGS-zdump.c = -fwrapv -Wno-strict-prototypes -DNOID $(tz-cflags) \
		 -DHAVE_GETTEXT
CFLAGS-zic.c = -Wno-strict-prototypes -DNOID $(tz-cflags) -DHAVE_GETTEXT
CFLAGS-ialloc.c = -Wno-strict-prototypes -DNOID -DHAVE_GETTEXT
CFLAGS-scheck.c = -Wno-strict-prototypes -DNOID -DHAVE_GETTEXT

# We have to make sure the data for testing the tz functions is available.
# Don't add leapseconds here since test-tz made checks that work only without
# leapseconds.
testdata = $(objpfx)testdata
define build-testdata
GCONV_PATH=${common-objpfx}iconvdata LANGUAGE=C LC_ALL=C \
  $(built-program-cmd) -d $(testdata) -y ./yearistype $<
endef

$(objpfx)test-tz.out: $(addprefix $(testdata)/, America/New_York Etc/UTC UTC)
$(objpfx)tst-timezone.out: $(addprefix $(testdata)/, \
				       Europe/Berlin Universal \
				       Australia/Melbourne \
				       America/Sao_Paulo Asia/Tokyo \
				       Europe/London)

test-tz-ENV = TZDIR=$(testdata)
tst-timezone-ENV = TZDIR=$(testdata)

# Note this must come second in the deps list for $(built-program-cmd) to work.
zic-deps = $(objpfx)zic $(leapseconds) yearistype

$(testdata)/America/New_York: northamerica $(zic-deps)
	$(build-testdata)
$(testdata)/Etc/UTC: etcetera $(zic-deps)
	$(build-testdata)
# Use a pattern rule to indicate the command produces both targets at once.
# Two separate targets built separately can collide if in parallel.
%/UTC %/Universal: simplebackw $(zic-deps) %/Etc/UTC
	$(build-testdata)
$(testdata)/%/Berlin $(testdata)/%/London: europe $(zic-deps)
	$(build-testdata)
$(testdata)/Australia/Melbourne: australasia $(zic-deps)
	$(build-testdata)
$(testdata)/America/Sao_Paulo: southamerica $(zic-deps)
	$(build-testdata)
$(testdata)/Asia/Tokyo: asia $(zic-deps)
	$(build-testdata)


$(objpfx)tzselect: tzselect.ksh $(common-objpfx)config.make
	sed -e 's%@KSH@%$(KSH)%g' \
	    -e 's%@TZDIR@%$(zonedir)%g' < $< > $@.new
	chmod 555 $@.new
	mv -f $@.new $@
