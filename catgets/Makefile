# Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
# not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#
#	Makefile for catgets.
#
subdir	:= catgets

headers		= nl_types.h
distribute	= catgetsinfo.h config.h xopen-msg.sed test1.msg
routines	= catgets open_catalog
others		= gencat
install-bin	= gencat
extra-objs	= $(gencat-modules:=.o)

tests = tst-catgets

gencat-modules	= xmalloc

# To find xmalloc.c
vpath %.c ../locale/programs


include ../Rules

$(objpfx)gencat: $(gencat-modules:%=$(objpfx)%.o)

catgets-CPPFLAGS := -DNLSPATH='"$(msgcatdir)/%L/%N:$(msgcatdir)/%L/LC_MESSAGES/%N:$(msgcatdir)/%l/%N:$(msgcatdir)/%l/LC_MESSAGES/%N:"' \
	    -DHAVE_CONFIG_H

generated = de.msg test1.cat test1.h
generated-dirs = de

tst-catgets-ENV = NLSPATH="$(objpfx)%l/%N.cat" LANG=de

ifneq ($(cross-compiling),yes)
tests: $(objpfx)de/libc.cat $(objpfx)test1.cat
# This test just checks whether the program produces any error or not.
# The result is not tested.
$(objpfx)test1.cat: test1.msg $(objpfx)gencat
	$(built-program-cmd) -H $(objpfx)test1.h $@ $<
$(objpfx)de/libc.cat: $(objpfx)de.msg $(objpfx)gencat
	-mkdir $(objpfx)de
	$(built-program-cmd) $@ $<
$(objpfx)tst-catgets.out: $(objpfx)de/libc.cat

# Generate a non-simple input file.
$(objpfx)de.msg: $(..)po/de.po
	sed -f xopen-msg.sed $< > $@
endif
