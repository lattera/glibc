# Copyright (C) 1991-1999, 2000, 2001 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#
#	Makefile for stdlib routines
#
subdir	:= stdlib

headers	:= stdlib.h alloca.h monetary.h fmtmsg.h ucontext.h sys/ucontext.h    \
	   inttypes.h stdint.h bits/wordsize.h bits/wchar.h

routines	:=							      \
	atof atoi atol atoll						      \
	abort								      \
	bsearch qsort msort						      \
	getenv putenv setenv secure-getenv				      \
	exit on_exit atexit cxa_atexit cxa_on_exit cxa_finalize		      \
	abs labs llabs							      \
	div ldiv lldiv							      \
	mblen mbstowcs mbtowc wcstombs wctomb				      \
	random random_r rand rand_r					      \
	drand48 erand48 lrand48 nrand48 mrand48 jrand48			      \
	srand48 seed48 lcong48						      \
	drand48_r erand48_r lrand48_r nrand48_r mrand48_r jrand48_r	      \
	srand48_r seed48_r lcong48_r					      \
	drand48-iter							      \
	strtol strtoul strtoll strtoull					      \
	strtol_l strtoul_l strtoll_l strtoull_l				      \
	strtof strtod strtold						      \
	strtof_l strtod_l strtold_l					      \
	system canonicalize						      \
	a64l l64a							      \
	rpmatch strfmon strfmon_l getsubopt xpg_basename fmtmsg		      \
	strtoimax strtoumax wcstoimax wcstoumax				      \
	getcontext setcontext makecontext swapcontext

distribute	:= exit.h grouping.h abort-instr.h isomac.c tst-fmtmsg.sh
test-srcs	:= tst-fmtmsg
tests		:= tst-strtol tst-strtod testmb testrand testsort testdiv \
		   test-canon test-canon2 tst-strtoll tst-environ	  \
		   tst-xpg-basename tst-random tst-bsearch tst-limits


# Several mpn functions from GNU MP are used by the strtod function.
mpn-routines := inlines add_n addmul_1 cmp divmod_1 divrem udiv_qrnnd \
		lshift rshift mod_1 mul mul_1 mul_n sub_n submul_1
mpn-headers = longlong.h gmp.h gmp-impl.h gmp-mparam.h asm-syntax.h

routines := $(strip $(routines) $(mpn-routines))	\
	    dbl2mpn ldbl2mpn				\
	    mpn2flt mpn2dbl mpn2ldbl
aux := mp_clz_tab fpioconst
distribute := $(distribute) $(mpn-headers) gen-mpn-copy fpioconst.h

generated += isomac isomac.out

CFLAGS-bsearch.c = $(exceptions)
CFLAGS-msort.c = $(exceptions)
CFLAGS-qsort.c = $(exceptions)

include ../Rules


ifdef gmp-srcdir

gmp-srcdir := $(firstword $(filter-out ..//%,$(..)$(gmp-srcdir) $(gmp-srcdir)))

# Copy the mpn source files we use from the GNU MP source directory.
# `gmp-srcdir' is set by doing `configure --with-gmp=DIR'.
# (Do not try this at home.  You need an as yet unreleased version of GNU MP.)

mpn-sysdep := $(addsuffix .c,$(mpn-routines)) \
	      $(addsuffix .S,$(mpn-routines)) \
	      $(addsuffix .s,$(mpn-routines)) gmp-mparam.h asm-syntax.h

mpn-try := $(addprefix $(gmp-srcdir)/mpn/*/,$(mpn-sysdep)) \
	   $(addprefix $(gmp-srcdir)/mpn/*/*/,$(mpn-sysdep))
mpn-found = $(wildcard $(mpn-try))

include mpn-copy.mk
%.mk: gen-%; sh $< > $@

mpn-copy-1 := $(patsubst $(gmp-srcdir)/mpn/%,$(sysdep_dir)/%,$(mpn-found))
mpn-copy-sysdep := $(mpn-copy-sysdep) $(mpn-copy-1)
mpn-copy-sysdep := $(filter $(addsuffix %,\
		     $(wildcard $(sort $(dir $(mpn-copy-sysdep))))), \
		    $(mpn-copy-sysdep))
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

mpn-copy = $(filter-out $(mpn-sysdep),$(mpn-headers))# mp_clz_tab.c)
$(mpn-copy): %: $(ignore gmp2glibc.sed) $(gmp-srcdir)/%; $(gmp2glibc)

.PHONY: copy-mpn clean-mpn mpn-stuff
mpn-stuff: $(mpn-stuff)
copy-mpn: $(mpn-stuff)
ifeq ($(with-cvs),yes)
	test ! -d CVS || cvs $(CVSOPTS) commit -m'Updated from $(gmp-srcdir)' $+
endif
clean-mpn:
	rm -f $(mpn-stuff)

endif

# Testdir has to be named stdlib and needs to be writable
test-canon-ARGS = --test-dir=${common-objpfx}stdlib

# Run a test on the header files we use.
tests: $(objpfx)isomac.out $(objpfx)tst-fmtmsg.out

$(objpfx)isomac.out: $(objpfx)isomac
	$(dir $<)$(notdir $<) '$(CC)' '-I../include -I.. $(+sysdep-includes)' > $<.out

$(objpfx)isomac: isomac.c
	$(native-compile)

$(objpfx)tst-fmtmsg.out: tst-fmtmsg.sh $(objpfx)tst-fmtmsg
	$(SHELL) -e $< $(common-objpfx) $(common-objpfx)stdlib/
