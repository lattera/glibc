# Copyright (C) 1995,96,97,98,99,2000, 2001 Free Software Foundation, Inc.
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
#	Specific makefile for libio.
#
subdir	:= libio

headers	:= stdio.h libio.h _G_config.h bits/stdio.h bits/stdio-lock.h \
	   stdio_ext.h

routines	:=							      \
	filedoalloc iofclose iofdopen iofflush iofgetpos iofgets iofopen      \
	iofopncook iofputs iofread iofsetpos ioftell wfiledoalloc	      \
	iofwrite iogetdelim iogetline iogets iopadn iopopen ioputs	      \
	ioseekoff ioseekpos iosetbuffer iosetvbuf iosprintf ioungetc	      \
	iovsprintf iovsscanf						      \
	iofgetpos64 iofopen64 iofsetpos64				      \
	fputwc fputwc_u getwc getwc_u getwchar getwchar_u iofgetws iofgetws_u \
	iofputws iofputws_u iogetwline iowpadn ioungetwc putwc putwc_u	      \
	putwchar putwchar_u putchar putchar_u fwprintf swprintf vwprintf      \
	wprintf wscanf fwscanf vwscanf vswprintf iovswscanf swscanf wgenops   \
	wstrops wfileops iofwide fwide					      \
									      \
	clearerr feof ferror fileno fputc freopen fseek getc getchar	      \
	memstream pclose putc putchar rewind setbuf setlinebuf vasprintf      \
	iovdprintf vscanf vsnprintf obprintf fcloseall fseeko ftello	      \
	freopen64 fseeko64 ftello64					      \
									      \
	__fbufsize __freading __fwriting __freadable __fwritable __flbf	      \
	__fpurge __fpending __fsetlocking				      \
									      \
	libc_fatal fmemopen

tests = tst_swprintf tst_wprintf tst_swscanf tst_wscanf tst_getwc tst_putwc   \
	tst_wprintf2 tst-widetext test-fmemopen
test-srcs = test-freopen

all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

ifeq ($(versioning),yes)
routines += oldiofopen oldiofdopen oldiofclose oldiopopen oldpclose \
	    oldtmpfile oldiofgetpos oldiofgetpos64 oldiofsetpos	    \
	    oldiofsetpos64
endif

ifneq (,$(filter %REENTRANT, $(defines)))
routines += clearerr_u feof_u ferror_u fputc_u getc_u getchar_u		      \
	    iofflush_u putc_u putchar_u peekc iofread_u iofwrite_u iofgets_u  \
	    iofputs_u

CPPFLAGS += -D_IO_MTSAFE_IO
endif

# Support for exception handling.
CFLAGS-genops.c = $(exceptions)
CFLAGS-wgenops.c = $(exceptions)
CFLAGS-fileops.c = $(exceptions)
CFLAGS-wfileops.c = $(exceptions)
# XXX Do we need filedoalloc and wfiledoalloc?  Others?

CFLAGS-tst_putwc.c = -DOBJPFX=\"$(objpfx)\"

tst_wprintf2-ARGS = "Some Text"

tst-widetext-ENV = LOCPATH=$(common-objpfx)localedata LANGUAGE=C

aux	:= fileops genops stdfiles stdio strops

ifeq ($(versioning),yes)
aux	+= oldfileops oldstdfiles
endif

shared-only-routines = oldiofopen oldiofdopen oldiofclose oldfileops	\
		       oldstdfiles oldiopopen oldpclose oldtmpfile	\
		       oldiofgetpos oldiofgetpos64 oldiofsetpos		\
		       oldiofsetpos64

distribute := iolibio.h libioP.h strfile.h Banner test-freopen.sh

include ../Rules

tests: $(objpfx)test-freopen.out

$(objpfx)test-freopen.out: test-freopen.sh $(objpfx)test-freopen
	$(SHELL) -e $< $(common-objpfx) $(common-objpfx)libio/
