# Copyright (C) 1991, 92, 93, 94, 95, 96, 97 Free Software Foundation, Inc.
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

headers	:= sys/uio.h iovec.h sys/ioctl.h ioctls.h ioctl-types.h \
	   sys/ptrace.h sys/file.h sys/dir.h sys/cdefs.h \
	   ar.h a.out.h libgen.h stab.h stab.def sgtty.h \
	   ttyent.h syslog.h sys/syslog.h paths.h sys/reboot.h \
	   sys/mman.h sys/param.h fstab.h mntent.h search.h err.h error.h\
	   sys/queue.h sysexits.h syscall.h sys/syscall.h sys/swap.h \
	   sys/select.h ustat.h sys/ustat.h ustatbits.h sys/sysinfo.h \
	   regexp.h selectbits.h

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
	    swapon swapoff mktemp mkstemp \
	    ualarm usleep \
	    gtty stty \
	    ptrace \
	    fstab mntent mntent_r \
	    utimes \
	    truncate ftruncate \
	    chflags fchflags \
	    insremque getttyent getusershell getpass ttyslot \
	    syslog syscall daemon \
	    mmap munmap mprotect msync madvise \
	    efgcvt efgcvt_r qefgcvt qefgcvt_r \
	    hsearch hsearch_r tsearch lsearch \
	    err error ustat \
	    getsysstats dirname regexp
aux := init-misc
distribute := bsd-compat.c
extra-objs := bsd-compat.o
install-lib := libbsd-compat.a libg.a
non-lib.a := libbsd-compat.a
gpl2lgpl := error.c error.h

tests := tst-dirname

include ../Rules


$(objpfx)libbsd-compat.a: $(objpfx)bsd-compat.o
	rm -f $@
	ln $< $@

$(objpfx)libg.a: $(dep-dummy-lib); $(make-dummy-lib)


CFLAGS-init-misc.c = -fkeep-inline-functions
