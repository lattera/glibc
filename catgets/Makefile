# Copyright (C) 1996-2000, 2002, 2003, 2004 Free Software Foundation, Inc.
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
#	Makefile for catgets.
#
subdir	:= catgets

headers		= nl_types.h
distribute	= catgetsinfo.h config.h xopen-msg.sed test1.msg test2.msg \
		  test-gencat.sh sample.SJIS
routines	= catgets open_catalog
others		= gencat
install-bin	= gencat
extra-objs	= $(gencat-modules:=.o)

tests = tst-catgets
test-srcs = test-gencat

gencat-modules	= xmalloc

# To find xmalloc.c
vpath %.c ../locale/programs


include ../Rules

$(objpfx)gencat: $(gencat-modules:%=$(objpfx)%.o)

catgets-CPPFLAGS := -DNLSPATH='"$(msgcatdir)/%L/%N:$(msgcatdir)/%L/LC_MESSAGES/%N:$(msgcatdir)/%l/%N:$(msgcatdir)/%l/LC_MESSAGES/%N:"' \
	    -DHAVE_CONFIG_H

CPPFLAGS-gencat = -DNOT_IN_libc

generated = de.msg test1.cat test1.h test2.cat test2.h sample.SJIS.cat \
	    test-gencat.h
generated-dirs = de

tst-catgets-ENV = NLSPATH="$(objpfx)%l/%N.cat" LANG=de

ifneq ($(cross-compiling),yes)
tests: $(objpfx)de/libc.cat $(objpfx)test1.cat $(objpfx)test2.cat \
       $(objpfx)test-gencat.out
# This test just checks whether the program produces any error or not.
# The result is not tested.
$(objpfx)test1.cat: test1.msg $(objpfx)gencat
	LC_ALL=hr_HR.ISO-8859-2 LOCPATH=$(common-objpfx)localedata \
	GCONV_PATH=$(common-objpfx)iconvdata \
	$(built-program-cmd) -H $(objpfx)test1.h $@ $<
$(objpfx)test2.cat: test2.msg $(objpfx)gencat
	LOCPATH=$(common-objpfx)localedata \
	GCONV_PATH=$(common-objpfx)iconvdata \
	$(built-program-cmd) -H $(objpfx)test2.h $@ $<
$(objpfx)de/libc.cat: $(objpfx)de.msg $(objpfx)gencat
	$(make-target-directory)
	LC_ALL=de_DE.ISO-8859-1 LOCPATH=$(common-objpfx)localedata \
	GCONV_PATH=$(common-objpfx)iconvdata $(built-program-cmd) $@ $<
$(objpfx)tst-catgets.out: $(objpfx)de/libc.cat

# Generate a non-simple input file.
$(objpfx)de.msg: $(..)po/de.po
	LC_ALL=C sed -f xopen-msg.sed $< > $@

$(objpfx)test-gencat.out: test-gencat.sh $(objpfx)test-gencat \
			  $(objpfx)sample.SJIS.cat
	$(SHELL) -e $< $(common-objpfx) '$(built-program-cmd)'

$(objpfx)sample.SJIS.cat: sample.SJIS $(objpfx)gencat
	GCONV_PATH=$(common-objpfx)iconvdata LC_ALL=C \
	$(built-program-cmd) -H $(objpfx)test-gencat.h < $(word 1,$^) > $@
endif
