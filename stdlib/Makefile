# Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#	Makefile for stdlib routines
#
subdir	:= stdlib

headers	:= stdlib.h alloca.h

routines	:=							      \
	atof atoi atol							      \
	abort								      \
	bsearch qsort msort						      \
	getenv putenv setenv						      \
	exit on_exit atexit						      \
	abs labs							      \
	div ldiv							      \
	mblen mbstowcs mbtowc wcstombs wctomb				      \
	random rand 							      \
	strtol strtoul strtoq strtouq					      \
	strtof strtod strtold						      \
	system

distribute	:= exit.h grouping.h
tests		:= tst-strtol tst-strtod testmb testrand testsort testdiv


# Several mpn functions from GNU MP are used by the strtod function.
mpn-routines := add_1 add_n addmul_1 cmp divmod divmod_1 udiv_qrnnd \
		lshift rshift mod_1 mul mul_1 mul_n sub_n submul_1 
mpn-headers = longlong.h gmp.h gmp-impl.h gmp-mparam.h asm-syntax.h

routines := $(strip $(routines) $(mpn-routines))	\
	    dbl2mpn ldbl2mpn				\
	    mpn2flt mpn2dbl mpn2ldbl
aux := mp_clz_tab fpioconst 
distribute := $(distribute) $(mpn-headers) gen-mpn-copy fpioconst.h


include ../Rules


ifdef gmp-srcdir

gmp-srcdir := $(firstword $(filter-out ..//%,$(..)$(gmp-srcdir) $(gmp-srcdir)))

# Copy the mpn source files we use from the GNU MP source directory.
# `gmp-srcdir' is set by doing `configure --with-gmp=DIR'.
# (Do not try this at home.  You need an as yet unreleased version of GNU MP.)

mpn-sysdep := $(addsuffix .c,$(mpn-routines)) \
	      $(addsuffix .S,$(mpn-routines)) \
	      $(addsuffix .s,$(mpn-routines)) gmp-mparam.h asm-syntax.h

mpn-try := $(addprefix $(gmp-srcdir)/mpn/*/,$(mpn-sysdep))
mpn-found := $(wildcard $(mpn-try))
mpn-found := $(filter-out $(patsubst %.S,%.s,$(filter %.s,$(mpn-found))),\
			  $(mpn-found))

include mpn-copy.mk
%.mk: gen-%; sh $< > $@

mpn-copy-1 := $(patsubst $(gmp-srcdir)/mpn/%,$(sysdep_dir)/%,$(mpn-found))
mpn-copy-sysdep := $(mpn-copy-sysdep) $(mpn-copy-1)
$(mpn-copy-1): $(sysdep_dir)/%: $(ignore gmp2glibc.sed) $(gmp-srcdir)/mpn/%
	$(gmp2glibc)

mpn-stuff = $(mpn-copy-sysdep) $(mpn-copy)

# chmod so I don't edit them by mistake.
define gmp2glibc
$(ignore sed -f $^ > $@-tmp)
cp $< $@-tmp
chmod a-w $@-tmp
mv -f $@-tmp $@
endef

mpn-copy = $(filter-out $(mpn-sysdep),$(mpn-headers) mp_clz_tab.c)
$(mpn-copy): %: $(ignore gmp2glibc.sed) $(gmp-srcdir)/%; $(gmp2glibc)

.PHONY: copy-mpn clean-mpn
copy-mpn: $(mpn-stuff)
clean-mpn:
	rm -f $(mpn-stuff)

endif
