# Copyright (C) 1991-1999,2000,01 Free Software Foundation, Inc.
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
#	Sub-makefile for misc portion of the library.
#

subdir	:= misc

headers	:= sys/uio.h bits/uio.h sys/ioctl.h bits/ioctls.h bits/ioctl-types.h \
	   sys/ptrace.h sys/file.h sys/dir.h sys/cdefs.h \
	   ar.h a.out.h libgen.h stab.h bits/stab.def sgtty.h \
	   ttyent.h syslog.h sys/syslog.h paths.h sys/reboot.h \
	   sys/mman.h sys/param.h fstab.h mntent.h search.h err.h error.h \
	   sys/queue.h sysexits.h syscall.h sys/syscall.h sys/swap.h \
	   sys/select.h ustat.h sys/ustat.h bits/ustat.h sys/sysinfo.h \
	   regexp.h bits/select.h bits/mman.h

routines := brk sbrk sstk ioctl \
	    readv writev \
	    setreuid setregid \
	    seteuid setegid \
	    getpagesize \
	    getdtsz \
	    gethostname sethostname getdomain setdomain \
	    select pselect \
	    acct chroot fsync sync fdatasync reboot \
	    gethostid sethostid \
	    revoke vhangup \
	    swapon swapoff mktemp mkstemp mkstemp64 mkdtemp \
	    ualarm usleep \
	    gtty stty \
	    ptrace \
	    fstab mntent mntent_r \
	    utimes \
	    truncate ftruncate truncate64 ftruncate64 \
	    chflags fchflags \
	    insremque getttyent getusershell getpass ttyslot \
	    syslog syscall daemon \
	    mmap mmap64 munmap mprotect msync madvise mincore \
	    mlock munlock mlockall munlockall \
	    efgcvt efgcvt_r qefgcvt qefgcvt_r \
	    hsearch hsearch_r tsearch lsearch \
	    err error ustat \
	    getsysstats dirname regexp \
	    getloadavg getclktck

distribute := device-nrs.h

include ../Makeconfig

aux := init-misc
ifeq ($(build-static),yes)
install-lib := libbsd-compat.a libg.a
endif
gpl2lgpl := error.c error.h

tests := tst-dirname tst-tsearch tst-fdset tst-efgcvt tst-mntent

CFLAGS-tsearch.c = $(exceptions)
CFLAGS-lsearch.c = $(exceptions)

include ../Rules

$(objpfx)libbsd-compat.a: $(dep-dummy-lib); $(make-dummy-lib)
$(objpfx)libg.a: $(dep-dummy-lib); $(make-dummy-lib)

CFLAGS-init-misc.c = -fkeep-inline-functions

ifeq ($(build-shared),yes)
$(objpfx)tst-tsearch: $(common-objpfx)math/libm.so$(libm.so-version)
else
$(objpfx)tst-tsearch: $(common-objpfx)math/libm.a
endif

ifeq ($(build-bounded),yes)
$(objpfx)tst-tsearch-bp: $(common-objpfx)math/libm_b.a
endif
