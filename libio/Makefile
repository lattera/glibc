# Copyright (C) 1995-2015 Free Software Foundation, Inc.
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
#	Specific makefile for libio.
#
subdir	:= libio

include ../Makeconfig

headers	:= stdio.h libio.h _G_config.h bits/stdio.h bits/stdio-lock.h \
	   bits/sys_errlist.h bits/stdio2.h bits/stdio-ldbl.h bits/libio-ldbl.h

routines	:=							      \
	filedoalloc iofclose iofdopen iofflush iofgetpos iofgets iofopen      \
	iofopncook iofputs iofread iofsetpos ioftell wfiledoalloc	      \
	iofwrite iogetdelim iogetline iogets iopadn iopopen ioputs	      \
	ioseekoff ioseekpos iosetbuffer iosetvbuf ioungetc		      \
	iovsprintf iovsscanf						      \
	iofgetpos64 iofopen64 iofsetpos64				      \
	fputwc fputwc_u getwc getwc_u getwchar getwchar_u iofgetws iofgetws_u \
	iofputws iofputws_u iogetwline iowpadn ioungetwc putwc putwc_u	      \
	putwchar putwchar_u putchar putchar_u fwprintf swprintf vwprintf      \
	wprintf wscanf fwscanf vwscanf vswprintf iovswscanf swscanf wgenops   \
	wstrops wfileops iofwide fwide wmemstream			      \
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
	tst_wprintf2 tst-widetext test-fmemopen tst-ext tst-ext2 \
	tst-fgetws tst-ungetwc1 tst-ungetwc2 tst-swscanf tst-sscanf	      \
	tst-mmap-setvbuf bug-ungetwc1 bug-ungetwc2 tst-atime tst-eof          \
	tst-freopen bug-rewind bug-rewind2 bug-ungetc bug-fseek \
	tst-mmap-eofsync tst-mmap-fflushsync bug-mmap-fflush \
	tst-mmap2-eofsync tst-mmap-offend bug-fopena+ bug-wfflush \
	bug-ungetc2 bug-ftell bug-ungetc3 bug-ungetc4 tst-fopenloc2 \
	tst-memstream1 tst-memstream2 \
	tst-wmemstream1 tst-wmemstream2 \
	bug-memstream1 bug-wmemstream1 \
	tst-setvbuf1 tst-popen1 tst-fgetwc bug-wsetpos tst-fseek \
	tst-fwrite-error tst-ftell-partial-wide tst-ftell-active-handler \
	tst-ftell-append tst-fputws
ifeq (yes,$(build-shared))
# Add test-fopenloc only if shared library is enabled since it depends on
# shared localedata objects.
tests += tst-fopenloc
endif
test-srcs = test-freopen

ifeq ($(build-shared),yes)
routines += oldiofopen oldiofdopen oldiofclose oldiopopen oldpclose \
	    oldtmpfile oldiofgetpos oldiofgetpos64 oldiofsetpos	    \
	    oldiofsetpos64
endif

ifeq (yes,$(libc-reentrant))
routines += clearerr_u feof_u ferror_u fputc_u getc_u getchar_u		      \
	    iofflush_u putc_u putchar_u peekc iofread_u iofwrite_u iofgets_u  \
	    iofputs_u
endif

CPPFLAGS += $(libio-mtsafe)

# Support for exception handling.
CFLAGS-fileops.c = $(exceptions)
CFLAGS-fputc.c = $(exceptions)
CFLAGS-fputwc.c = $(exceptions)
CFLAGS-freopen64.c = $(exceptions)
CFLAGS-freopen.c = $(exceptions)
CFLAGS-fseek.c = $(exceptions)
CFLAGS-fseeko64.c = $(exceptions)
CFLAGS-fseeko.c = $(exceptions)
CFLAGS-ftello64.c = $(exceptions)
CFLAGS-ftello.c = $(exceptions)
CFLAGS-fwide.c = $(exceptions)
CFLAGS-genops.c = $(exceptions)
CFLAGS-getc.c = $(exceptions)
CFLAGS-getchar.c = $(exceptions)
CFLAGS-getwc.c = $(exceptions)
CFLAGS-getwchar.c = $(exceptions)
CFLAGS-iofclose.c = $(exceptions)
CFLAGS-iofflush.c = $(exceptions)
CFLAGS-iofgetpos64.c = $(exceptions)
CFLAGS-iofgetpos.c = $(exceptions)
CFLAGS-iofgets.c = $(exceptions)
CFLAGS-iofgetws.c = $(exceptions)
CFLAGS-iofputs.c = $(exceptions)
CFLAGS-iofputws.c = $(exceptions)
CFLAGS-iofread.c = $(exceptions)
CFLAGS-iofsetpos64.c = $(exceptions)
CFLAGS-iofsetpos.c = $(exceptions)
CFLAGS-ioftell.c = $(exceptions)
CFLAGS-iofwrite.c = $(exceptions)
CFLAGS-iogetdelim.c = $(exceptions)
CFLAGS-iogetline.c = $(exceptions)
CFLAGS-iogets.c = $(exceptions)
CFLAGS-iogetwline.c = $(exceptions)
CFLAGS-ioputs.c = $(exceptions)
CFLAGS-ioseekoff.c = $(exceptions)
CFLAGS-ioseekpos.c = $(exceptions)
CFLAGS-iosetbuffer.c = $(exceptions)
CFLAGS-iosetvbuf.c = $(exceptions)
CFLAGS-ioungetc.c = $(exceptions)
CFLAGS-ioungetwc.c = $(exceptions)
CFLAGS-oldfileops.c = $(exceptions)
CFLAGS-oldiofclose.c = $(exceptions)
CFLAGS-oldiofgetpos64.c = $(exceptions)
CFLAGS-oldiofgetpos.c = $(exceptions)
CFLAGS-oldiofsetpos64.c = $(exceptions)
CFLAGS-oldiofsetpos.c = $(exceptions)
CFLAGS-peekc.c = $(exceptions)
CFLAGS-putc.c = $(exceptions)
CFLAGS-putchar.c = $(exceptions)
CFLAGS-putwc.c = $(exceptions)
CFLAGS-putwchar.c = $(exceptions)
CFLAGS-rewind.c = $(exceptions)
CFLAGS-wfileops.c = $(exceptions)
CFLAGS-wgenops.c = $(exceptions)
CFLAGS-oldiofopen.c = $(exceptions)
CFLAGS-iofopen.c = $(exceptions)
CFLAGS-iofopen64.c = $(exceptions)
CFLAGS-oldtmpfile.c = $(exceptions)
# XXX Do we need filedoalloc and wfiledoalloc?  Others?

CFLAGS-tst_putwc.c = -DOBJPFX=\"$(objpfx)\"

tst_wprintf2-ARGS = "Some Text"

tst-fopenloc-ENV = MALLOC_TRACE=$(objpfx)tst-fopenloc.mtrace

generated += tst-fopenloc.mtrace tst-fopenloc.check

aux	:= fileops genops stdfiles stdio strops

ifeq ($(build-shared),yes)
aux	+= oldfileops oldstdfiles
endif

shared-only-routines = oldiofopen oldiofdopen oldiofclose oldfileops	\
		       oldstdfiles oldiopopen oldpclose oldtmpfile	\
		       oldiofgetpos oldiofgetpos64 oldiofsetpos		\
		       oldiofsetpos64

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)test-freopen.out
ifeq (yes,$(build-shared))
# Run tst-fopenloc-cmp.out and tst-openloc-mem.out only if shared
# library is enabled since they depend on tst-fopenloc.out.
tests-special += $(objpfx)tst-fopenloc-cmp.out $(objpfx)tst-fopenloc-mem.out
endif
endif

include ../Rules

$(objpfx)test-freopen.out: test-freopen.sh $(objpfx)test-freopen
	$(SHELL) $< $(common-objpfx) '$(test-program-prefix)'	\
	$(common-objpfx)libio/; \
	$(evaluate-test)

$(objpfx)tst-fopenloc-cmp.out: ../iconvdata/testdata/ISO-8859-1..UTF8 \
			       $(objpfx)tst-fopenloc.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-fopenloc-mem.out: $(objpfx)tst-fopenloc.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-fopenloc.mtrace > $@; \
	$(evaluate-test)
