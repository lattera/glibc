# Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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
#	Makefile for locales.
#
subdir	:= locale

headers		= locale.h
distribute	= localeinfo.h categories.def \
		  $(localedef-modules:=.c) $(locale-modules:=.c) \
		  $(lib-modules:=.c) config.h hash.h iso-4217.def \
		  keyword.gperf keyword.h localedef.h token.h
routines	= setlocale loadlocale localeconv nl_langinfo
categories	= ctype messages monetary numeric time collate
aux		= $(categories:%=lc-%) $(categories:%=C-%)
others		= localedef locale
install-bin	= localedef locale
extra-objs	= $(localedef-modules:=.o) $(locale-modules:=.o) \
		  $(lib-modules:=.o)

localedef-modules	:= charmap locfile-lex locfile-parse ctype \
			   monetary messages collate numeric
locale-modules		:= ctypedump
lib-modules		:= error hash xmalloc


GPERF = gperf
GPERFFLAGS = -acCgopt -k1,2,5,$$

include ../Rules

keyword.h: keyword.gperf
	$(GPERF) $(GPERFFLAGS) $< > $@.new
	mv -f $@.new $@

$(objpfx)localedef: $(localedef-modules:%=$(objpfx)%.o)
$(objpfx)locale: $(locale-modules:%=$(objpfx)%.o)
$(objpfx)localedef $(objpfx)locale: $(lib-modules:%=$(objpfx)%.o)

CPPFLAGS += -DLOCALE_PATH='"$(localedir)"' -DCHARMAP_PATH='"$(nlsdir)/charmap"'
