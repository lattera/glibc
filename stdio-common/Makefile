# Copyright (C) 1991-1999, 2000, 2001 Free Software Foundation, Inc.
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
#	Specific makefile for stdio-common.
#
subdir	:= stdio-common

headers	:= printf.h stdio_ext.h

routines	:=							      \
	ctermid cuserid							      \
	_itoa _itowa itoa-digits itoa-udigits itowa-digits		      \
	vfprintf vprintf printf_fp reg-printf printf-prs printf_fphex	      \
	printf_size fprintf printf snprintf sprintf asprintf dprintf	      \
	vfscanf								      \
	fscanf scanf sscanf						      \
	perror psignal							      \
	tmpfile tmpfile64 tmpnam tmpnam_r tempnam tempname		      \
	getline getw putw						      \
	remove rename							      \
	lockfile

include ../Makeconfig

ifeq ($(stdio),libio)
routines += vfwprintf vfwscanf
endif

install-others := $(inst_includedir)/bits/stdio_lim.h

aux	:= errlist siglist
distribute := _itoa.h _itowa.h _i18n_number.h \
	      printf-parse.h stdio_lim.h.in tst-unbputc.sh tst-printf.sh

tests := tstscanf test_rdwr test-popen tstgetln test-fseek \
	 temptest tst-fileno test-fwrite tst-ungetc tst-ferror \
	 xbug errnobug \
	 bug1 bug2 bug3 bug4 bug5 bug6 bug7 bug8 bug9 bug10 bug11 bug12 bug13 \
	 tfformat tiformat tllformat tstdiomisc tst-printfsz tst-wc-printf \
	 scanf1 scanf2 scanf3 scanf4 scanf5 scanf7 scanf8 scanf9 scanf10 \
	 scanf12 tst-tmpnam tst-cookie tst-obprintf tst-sscanf tst-swprintf \
	 tst-fseek tst-fmemopen test-vfprintf tst-gets

test-srcs = tst-unbputc tst-printf

include ../Rules

ifeq ($(cross-compiling),no)
.PHONY: do-tst-unbputc do-tst-printf
tests: do-tst-unbputc do-tst-printf

do-tst-unbputc: $(objpfx)tst-unbputc.out
$(objpfx)tst-unbputc.out: $(objpfx)tst-unbputc tst-unbputc.sh
	$(SHELL) -e tst-unbputc.sh $(common-objpfx) '$(run-program-prefix)'

do-tst-printf: $(objpfx)tst-printf.out
$(objpfx)tst-printf.out: $(objpfx)tst-printf tst-printf.sh
	$(SHELL) -e tst-printf.sh $(common-objpfx) '$(run-program-prefix)'
endif

CFLAGS-vfprintf.c = -Wno-uninitialized
CFLAGS-tst-printf.c = -Wno-format
CFLAGS-tstdiomisc.c = -Wno-format
CFLAGS-scanf4.c = -Wno-format
CFLAGS-scanf7.c = -Wno-format
CFLAGS-tst-printfsz.c = -Wno-format

tst-sscanf-ENV = LOCPATH=$(common-objpfx)localedata
tst-swprintf-ENV = LOCPATH=$(common-objpfx)localedata
test-vfprintf-ENV = LOCPATH=$(common-objpfx)localedata

$(inst_includedir)/bits/stdio_lim.h: $(common-objpfx)bits/stdio_lim.h
	$(do-install)

ifeq ($(stdio),libio)
ifneq (,$(filter %REENTRANT, $(defines)))
CPPFLAGS += -D_IO_MTSAFE_IO
endif
endif
