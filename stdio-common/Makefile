# Copyright (C) 1991, 92, 93, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
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
#	Specific makefile for stdio-common.
#
subdir	:= stdio-common

headers	:= printf.h

routines	:=							      \
	ctermid cuserid							      \
	_itoa itoa-digits						      \
	vfprintf vprintf printf_fp reg-printf printf-prs printf_fphex	      \
	printf_size fprintf printf snprintf sprintf asprintf dprintf	      \
	vfscanf 							      \
	fscanf scanf sscanf						      \
	perror psignal							      \
	tmpfile tmpfile64 tmpnam tmpnam_r tempnam tempname		      \
	getline getw putw						      \
	remove rename							      \
	lockfile

install-others := $(inst_includedir)/bits/stdio_lim.h

aux	:= errlist siglist
distribute := _itoa.h printf-parse.h stdio_lim.h.in

tests := tst-printf tstscanf test_rdwr test-popen tstgetln test-fseek \
	 temptest tst-fileno test-fwrite tst-ungetc tst-ferror \
	 xbug errnobug \
	 bug1 bug2 bug3 bug4 bug5 bug6 bug7 bug8 bug9 bug10 bug11 bug12 \
	 tfformat tiformat tstdiomisc tst-printfsz tst-wc-printf \
	 scanf1 scanf2 scanf3 scanf4 scanf5 scanf7 scanf8 scanf9 scanf10

include ../Rules

CFLAGS-vfprintf.c = -Wno-uninitialized
CFLAGS-_itoa.c = -Wno-unused
CFLAGS-tst-printf.c = -Wno-format
CFLAGS-tstdiomisc.c = -Wno-format
CFLAGS-scanf4.c = -Wno-format
CFLAGS-scanf7.c = -Wno-format
CFLAGS-tst-printfsz.c = -Wno-format

$(inst_includedir)/bits/stdio_lim.h: $(common-objpfx)bits/stdio_lim.h
	$(make-target-directory)
	if test -r $@ && cmp -s $< $@; \
	then echo 'gnu/lib-names.h unchanged'; \
	else $(INSTALL_DATA) $< $@; fi

ifeq ($(stdio),libio)
ifneq (,$(filter %REENTRANT, $(defines)))
CPPFLAGS += -D_IO_MTSAFE_IO
endif
endif
