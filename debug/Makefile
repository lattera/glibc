# Copyright (C) 1998, 1999, 2000, 2001, 2004 Free Software Foundation, Inc.
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
#	Sub-makefile for debug portion of the library.
#
subdir	:= debug

headers	:= execinfo.h
distribute = sigcontextinfo.h register-dump.h frame.h

routines := backtrace backtracesyms backtracesymsfd noophooks \
	    memcpy_chk memmove_chk mempcpy_chk memset_chk stpcpy_chk \
	    strcat_chk strcpy_chk strncat_chk strncpy_chk \
	    sprintf_chk vsprintf_chk snprintf_chk vsnprintf_chk \
	    printf_chk fprintf_chk vprintf_chk vfprintf_chk \
	    gets_chk chk_fail readonly-area

CFLAGS-backtrace.c = -fno-omit-frame-pointer
CFLAGS-sprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-snprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-vsprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-vsnprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-printf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-vprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-vfprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-gets_chk.c = -D_IO_MTSAFE_IO $(exceptions)

tests = backtrace-tst tst-chk1 tst-chk2 tst-chk3 \
	test-strcpy_chk test-stpcpy_chk

extra-libs = libSegFault libpcprofile
extra-libs-others = $(extra-libs)

libSegFault-routines = segfault
libSegFault-inhibit-o = $(filter-out .os,$(object-suffixes))

libpcprofile-routines = pcprofile
libpcprofile-inhibit-o = $(filter-out .os,$(object-suffixes))

others = pcprofiledump
install-bin = pcprofiledump
install-bin-script = xtrace

include ../Makeconfig

distribute += catchsegv.sh xtrace.sh
ifeq ($(elf),yes)
ifeq ($(build-shared),yes)
install-bin-script += catchsegv
endif
endif
generated = catchsegv xtrace

include ../Rules

$(objpfx)catchsegv: catchsegv.sh $(common-objpfx)soversions.mk \
		    $(common-objpfx)config.make
	slibpfx=`echo $(slibdir)|sed 's/lib\(64\|\)$$/\\\\\\\\$$LIB/'`; \
	sed -e 's|@VERSION@|$(version)|' -e "s|@SLIB@|$$slibpfx|" $< > $@.new
	chmod 555 $@.new
	mv -f $@.new $@

$(objpfx)pcprofiledump: $(objpfx)pcprofiledump.o

$(objpfx)xtrace: xtrace.sh
	rm -f $@.new
	sed -e 's|@BASH@|$(BASH)|' -e 's|@VERSION@|$(version)|' \
	    -e 's|@SLIBDIR@|$(slibdir)|' -e 's|@BINDIR@|$(bindir)|' $^ > $@.new \
	&& rm -f $@ && mv $@.new $@ && chmod +x $@

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libSegFault.so: $(common-objpfx)libc.so \
			 $(common-objpfx)libc_nonshared.a \
			 $(elf-objpfx)$(rtld-installed-name)
$(objpfx)libpcprofile.so: $(common-objpfx)libc.so \
			  $(common-objpfx)libc_nonshared.a \
			  $(elf-objpfx)$(rtld-installed-name)
