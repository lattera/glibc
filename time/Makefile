# Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Makefile for time routines
#
subdir	:= time

headers	:= time.h sys/time.h sys/timeb.h
distribute := tzfile.h private.h scheck.c ialloc.c yearistype
extra-objs = scheck.o ialloc.o zonenames $(tzfiles:%=z.%)

routines	:= offtime asctime clock ctime difftime gmtime	\
		   localtime mktime strftime time tzset tzfile	\
		   gettimeofday settimeofday adjtime		\
		   getitimer setitimer				\
		   stime dysize timegm ftime

others	:= ap zdump zic
tests	:= test_time clocktest

tzfiles := africa antarctica asia australasia europe northamerica \
	   southamerica etcetera factory systemv backward
# pacificnew doesn't compile; if it is to be used, it should be included in
# northamerica.
distribute := $(distribute) $(tzfiles) leapseconds pacificnew

install-sbin := zic zdump

include ../Makeconfig	# Get objpfx defined so we can use it below.

# zonenames uses this variable.
define nl


endef
include $(objpfx)zonenames

# Make these absolute file names.
installed-localtime-file := $(firstword $(filter /%,$(localtime-file)) \
					$(addprefix $(zonedir)/, \
						    $(localtime-file)))
installed-posixrules-file := $(firstword $(filter /%,$(posixrules-file)) \
					 $(addprefix $(zonedir)/, \
						     $(posixrules-file)))

ifeq ($(cross-compiling),no)
# Don't try to install the zoneinfo files since we can't run zic.
install-others = $(addprefix $(zonedir)/,$(zonenames)) \
		 $(installed-localtime-file) $(installed-posixrules-file)
endif

include ../Rules


$(tzfiles:%=$(objpfx)z.%): $(objpfx)z.%: % Makefile
# Kludge alert: we use an implicit rule (in what we are generating here)
# because that is the only way to tell Make that the one command builds all
# the files.
	(echo 'define $*-zones'						;\
	 awk '$$1 == "Zone" { print $$2 } $$1 == "Link" { print $$3 }' $^;\
	 echo 'endef'							;\
	 echo '$*-zones := $$(subst $$(nl), ,$$($*-zones))'		;\
	 echo 'ifdef $*-zones'						;\
	 echo '$$(addprefix $$(datadir)/zone%/,$$($*-zones)): \'	;\
	 echo '$< $$(objpfx)zic leapseconds yearistype'			;\
	 echo '	$$(tzcompile)'						;\
	 echo 'endif'							;\
	 echo 'zonenames := $$(zonenames) $$($*-zones)'			;\
	) > $@.new
	mv $@.new $@
$(objpfx)zonenames: Makefile
	(for file in $(tzfiles); do \
	   echo "include \$$(objpfx)z.$$file"; \
	 done) > $@.new
	mv $@.new $@

.PHONY: echo-zonenames
echo-zonenames: zonenames
	@echo 'Known zones: $(zonenames)'


# Although $(zonedir) gets compiled into zic, it is useful to always
# specify it with -d on the command line so that it can be overridden on
# the command line of `make install' (e.g., "make install prefix=/foo").
zic-cmd = $(built-program-cmd) -d $(zonedir)
define tzcompile
$(zic-cmd) -L $(word 3,$^) -y $(dir $(word 4,$^))$(notdir $(word 4,$^)) $<
endef

ifdef localtime
$(installed-localtime-file): $(zonedir)/$(localtime) $(objpfx)zic
	$(zic-cmd) -l $(localtime)
endif
ifdef posixrules
$(installed-posixrules-file): $(zonedir)/$(posixrules) $(objpfx)zic
	$(zic-cmd) -p $(posixrules)
endif


$(objpfx)zic: $(objpfx)scheck.o $(objpfx)ialloc.o

$(objpfx)tzfile.o: tzfile.c; $(tz-cc)
$(objpfx)zic.o: zic.c; $(tz-cc)

# Some versions of GNU make have a bug with backslashes in define directives.
tz-cc = $(COMPILE.c) $(+gcc-nowarn) \
	     -DTZDIR='"$(zonedir)"' \
	     -DTZDEFAULT='"$(localtime-file)"' \
	     -DTZDEFRULES='"$(posixrules-file)"' \
	     $< $(OUTPUT_OPTION)

CFLAGS-zdump.c = -Wno-strict-prototypes -DNOID
CFLAGS-zic.c = -Wno-strict-prototypes -DNOID
CFLAGS-ialloc.c = -Wno-strict-prototypes -DNOID
CFLAGS-scheck.c = -Wno-strict-prototypes -DNOID
