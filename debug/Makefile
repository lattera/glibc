# Copyright (C) 1998-2001,2004-2008, 2009, 2011, 2012 Free Software Foundation, Inc.
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
#	Sub-makefile for debug portion of the library.
#
subdir	:= debug

headers	:= execinfo.h
distribute = sigcontextinfo.h register-dump.h frame.h

routines  = backtrace backtracesyms backtracesymsfd noophooks \
	    memcpy_chk memmove_chk mempcpy_chk memset_chk stpcpy_chk \
	    strcat_chk strcpy_chk strncat_chk strncpy_chk stpncpy_chk \
	    sprintf_chk vsprintf_chk snprintf_chk vsnprintf_chk \
	    printf_chk fprintf_chk vprintf_chk vfprintf_chk \
	    gets_chk chk_fail readonly-area fgets_chk fgets_u_chk \
	    read_chk pread_chk pread64_chk recv_chk recvfrom_chk \
	    readlink_chk readlinkat_chk getwd_chk getcwd_chk \
	    realpath_chk ptsname_r_chk fread_chk fread_u_chk \
	    wctomb_chk wcscpy_chk wmemcpy_chk wmemmove_chk wmempcpy_chk \
	    wcpcpy_chk wcsncpy_chk wcscat_chk wcsncat_chk wmemset_chk \
	    wcpncpy_chk \
	    swprintf_chk vswprintf_chk wprintf_chk fwprintf_chk \
	    vwprintf_chk vfwprintf_chk fgetws_chk fgetws_u_chk \
	    confstr_chk getgroups_chk ttyname_r_chk getlogin_r_chk \
	    gethostname_chk getdomainname_chk wcrtomb_chk mbsnrtowcs_chk \
	    wcsnrtombs_chk mbsrtowcs_chk wcsrtombs_chk mbstowcs_chk \
	    wcstombs_chk asprintf_chk vasprintf_chk dprintf_chk \
	    vdprintf_chk obprintf_chk \
	    longjmp_chk ____longjmp_chk \
	    fdelt_chk poll_chk ppoll_chk \
	    stack_chk_fail fortify_fail \
	    $(static-only-routines)
static-only-routines := warning-nop stack_chk_fail_local

CFLAGS-backtrace.c = -fno-omit-frame-pointer
CFLAGS-sprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-snprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-vsprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-vsnprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-asprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-vasprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-obprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-dprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-vdprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-printf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-vprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-vfprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-gets_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fgets_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fgets_u_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fread_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fread_u_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-swprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-vswprintf_chk.c = -D_IO_MTSAFE_IO
CFLAGS-wprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fwprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-vwprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-vfwprintf_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fgetws_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-fgetws_u_chk.c = -D_IO_MTSAFE_IO $(exceptions)
CFLAGS-read_chk.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pread_chk.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pread64_chk.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-recv_chk.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-recvfrom_chk.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-tst-longjmp_chk.c = -fexceptions -fasynchronous-unwind-tables \
			   -D_FORTIFY_SOURCE=1
CFLAGS-tst-longjmp_chk2.c = -fexceptions -fasynchronous-unwind-tables \
			    -D_FORTIFY_SOURCE=1

# We know these tests have problems with format strings, this is what
# we are testing.  Disable that warning.
CFLAGS-tst-chk1.c = -Wno-format
CFLAGS-tst-chk2.c = -Wno-format
CFLAGS-tst-chk3.c = -Wno-format
CFLAGS-tst-chk4.cc = -Wno-format
CFLAGS-tst-chk5.cc = -Wno-format
CFLAGS-tst-chk6.cc = -Wno-format
CFLAGS-tst-lfschk1.c = -Wno-format
CFLAGS-tst-lfschk2.c = -Wno-format
CFLAGS-tst-lfschk3.c = -Wno-format
CFLAGS-tst-lfschk4.cc = -Wno-format
CFLAGS-tst-lfschk5.cc = -Wno-format
CFLAGS-tst-lfschk6.cc = -Wno-format
tst-chk1-ENV = LOCPATH=$(common-objpfx)localedata
tst-chk2-ENV = LOCPATH=$(common-objpfx)localedata
tst-chk3-ENV = LOCPATH=$(common-objpfx)localedata
tst-chk4-ENV = LOCPATH=$(common-objpfx)localedata
tst-chk5-ENV = LOCPATH=$(common-objpfx)localedata
tst-chk6-ENV = LOCPATH=$(common-objpfx)localedata
tst-lfschk1-ENV = LOCPATH=$(common-objpfx)localedata
tst-lfschk2-ENV = LOCPATH=$(common-objpfx)localedata
tst-lfschk3-ENV = LOCPATH=$(common-objpfx)localedata
tst-lfschk4-ENV = LOCPATH=$(common-objpfx)localedata
tst-lfschk5-ENV = LOCPATH=$(common-objpfx)localedata
tst-lfschk6-ENV = LOCPATH=$(common-objpfx)localedata
LDFLAGS-tst-chk4 = -lstdc++
LDFLAGS-tst-chk5 = -lstdc++
LDFLAGS-tst-chk6 = -lstdc++
LDFLAGS-tst-lfschk4 = -lstdc++
LDFLAGS-tst-lfschk5 = -lstdc++
LDFLAGS-tst-lfschk6 = -lstdc++

tests = backtrace-tst tst-longjmp_chk tst-chk1 tst-chk2 tst-chk3 \
	tst-lfschk1 tst-lfschk2 tst-lfschk3 test-strcpy_chk test-stpcpy_chk \
	tst-chk4 tst-chk5 tst-chk6 tst-lfschk4 tst-lfschk5 tst-lfschk6 \
	tst-longjmp_chk2

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
ifeq ($(build-shared),yes)
install-bin-script += catchsegv
endif
generated = catchsegv xtrace

include ../Rules

sLIBdir := $(shell echo $(slibdir) | sed 's,lib\(\|64\)$$,\\\\$$LIB,')

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
	    -e 's|@SLIBDIR@|$(sLIBdir)|' -e 's|@BINDIR@|$(bindir)|' $^ > $@.new \
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
