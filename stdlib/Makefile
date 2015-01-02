# Copyright (C) 1991-2015 Free Software Foundation, Inc.
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
#	Makefile for stdlib routines
#
subdir	:= stdlib

include ../Makeconfig

headers	:= stdlib.h bits/stdlib.h bits/stdlib-ldbl.h bits/stdlib-float.h      \
	   monetary.h bits/monetary-ldbl.h				      \
	   inttypes.h stdint.h bits/wordsize.h				      \
	   errno.h sys/errno.h bits/errno.h				      \
	   ucontext.h sys/ucontext.h					      \
	   alloca.h fmtmsg.h						      \
	   bits/stdlib-bsearch.h

routines	:=							      \
	atof atoi atol atoll						      \
	abort								      \
	bsearch qsort msort						      \
	getenv putenv setenv secure-getenv				      \
	exit on_exit atexit cxa_atexit cxa_finalize old_atexit		      \
	quick_exit at_quick_exit cxa_at_quick_exit cxa_thread_atexit_impl     \
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
aux =	grouping groupingwc tens_in_limb

# These routines will be omitted from the libc shared object.
# Instead the static object files will be included in a special archive
# linked against when the shared library will be used.
static-only-routines = atexit at_quick_exit

test-srcs	:= tst-fmtmsg
tests		:= tst-strtol tst-strtod testmb testrand testsort testdiv   \
		   test-canon test-canon2 tst-strtoll tst-environ	    \
		   tst-xpg-basename tst-random tst-random2 tst-bsearch	    \
		   tst-limits tst-rand48 bug-strtod tst-setcontext	    \
		   test-a64l tst-qsort tst-system testmb2 bug-strtod2	    \
		   tst-atof1 tst-atof2 tst-strtod2 tst-strtod3 tst-rand48-2 \
		   tst-makecontext tst-strtod4 tst-strtod5 tst-qsort2	    \
		   tst-makecontext2 tst-strtod6 tst-unsetenv1		    \
		   tst-makecontext3 bug-getcontext bug-fmtmsg1		    \
		   tst-secure-getenv tst-strtod-overflow tst-strtod-round   \
		   tst-tininess tst-strtod-underflow tst-tls-atexit
tests-static	:= tst-secure-getenv

modules-names	= tst-tls-atexit-lib

ifeq ($(build-shared),yes)
tests		+= tst-putenv
endif

# Several mpn functions from GNU MP are used by the strtod function.
mpn-routines := inlines add_n addmul_1 cmp divmod_1 divrem udiv_qrnnd \
		lshift rshift mod_1 mul mul_1 mul_n sub_n submul_1
mpn-headers = longlong.h gmp.h gmp-impl.h gmp-mparam.h asm-syntax.h

routines := $(strip $(routines) $(mpn-routines))	\
	    dbl2mpn ldbl2mpn				\
	    mpn2flt mpn2dbl mpn2ldbl
aux += fpioconst mp_clz_tab

tests-extras += tst-putenvmod
extra-test-objs += tst-putenvmod.os

generated += isomac isomac.out tst-putenvmod.so

CFLAGS-bsearch.c = $(uses-callbacks)
CFLAGS-msort.c = $(uses-callbacks)
CFLAGS-qsort.c = $(uses-callbacks)
CFLAGS-system.c = -fexceptions
CFLAGS-system.os = -fomit-frame-pointer
CFLAGS-fmtmsg.c = -fexceptions

CFLAGS-strfmon.c = $(libio-mtsafe)
CFLAGS-strfmon_l.c = $(libio-mtsafe)

CFLAGS-tst-bsearch.c = $(stack-align-test-flags)
CFLAGS-tst-qsort.c = $(stack-align-test-flags)
CFLAGS-tst-makecontext2.c = $(stack-align-test-flags)

# Run a test on the header files we use.
tests-special += $(objpfx)isomac.out

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)tst-fmtmsg.out
endif

include ../Rules

# Testdir has to be named stdlib and needs to be writable
test-canon-ARGS = --test-dir=${common-objpfx}stdlib

bug-fmtmsg1-ENV = SEV_LEVEL=foo,11,newsev

$(objpfx)isomac.out: $(objpfx)isomac
	$(dir $<)$(notdir $<) '$(CC)'  \
	'-I../include $(+sysdep-includes) $(sysincludes) -I..' > $<.out; \
	$(evaluate-test)

isomac-CFLAGS = -O
$(objpfx)isomac: isomac.c
	$(native-compile)

$(objpfx)tst-fmtmsg.out: tst-fmtmsg.sh $(objpfx)tst-fmtmsg
	$(SHELL) $< $(common-objpfx) '$(test-program-prefix-before-env)' \
		 '$(run-program-env)' '$(test-program-prefix-after-env)' \
		 $(common-objpfx)stdlib/; \
	$(evaluate-test)

$(objpfx)tst-putenv: $(objpfx)tst-putenvmod.so
LDFLAGS-tst-putenv = $(no-as-needed)

$(objpfx)tst-putenvmod.so: $(objpfx)tst-putenvmod.os $(link-libc-deps)
	$(build-module)
libof-tst-putenvmod = extramodules

$(objpfx)bug-getcontext: $(libm)
$(objpfx)tst-strtod-round: $(libm)
$(objpfx)tst-tininess: $(libm)
$(objpfx)tst-strtod-underflow: $(libm)
$(objpfx)tst-strtod6: $(libm)

tst-tls-atexit-lib.so-no-z-defs = yes

$(objpfx)tst-tls-atexit: $(shared-thread-library) $(libdl)
$(objpfx)tst-tls-atexit.out: $(objpfx)tst-tls-atexit-lib.so
