# Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Sub-makefile for misc portion of the library.
#

subdir	:= misc

headers	:= sys/uio.h sys/ioctl.h sys/ptrace.h ioctls.h sys/file.h	\
	   a.out.h nlist.h stab.h stab.def sgtty.h sys/dir.h sys/cdefs.h \
	   ttyent.h syscall.h syslog.h sys/syslog.h paths.h sys/reboot.h \
	   sys/mman.h sys/param.h fstab.h mntent.h search.h utmp.h	 \
	   err.h error.h queue.h

routines := brk sbrk sstk ioctl \
	    readv writev \
	    setreuid setregid \
	    seteuid setegid \
	    getpagesize \
	    getdtsz \
	    gethostname sethostname getdomain setdomain \
	    select \
	    acct chroot fsync sync reboot \
	    gethostid sethostid \
	    revoke vhangup mknod \
	    swapon mktemp mkstemp \
	    ualarm usleep \
	    gtty stty \
	    ptrace nlist \
	    fstab mntent \
	    utimes \
	    truncate ftruncate \
	    chflags fchflags \
	    insremque getttyent getusershell getpass ttyslot \
	    syslog syscall daemon \
	    mmap munmap mprotect msync madvise \
	    efgcvt efgcvt_r \
	    hsearch hsearch_r tsearch \
	    err error
aux := progname init-misc
distribute := bsd-compat.c
extra-objs := bsd-compat.o
install-lib := libbsd-compat.a libg.a
non-lib.a := libbsd-compat.a
gpl2lgpl := error.c error.h

# Build the -lutil library with these extra functions.
extra-libs	:= libutil
libutil-routines:= login login_tty logout logwtmp pty


include ../Rules


$(objpfx)libbsd-compat.a: $(objpfx)bsd-compat.o
	rm -f $@
	ln $< $@

$(objpfx)libg.a: $(dep-dummy-lib); $(make-dummy-lib)
