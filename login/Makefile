# Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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
#	Sub-makefile for login portion of the library.
#

subdir	:= login

headers	:= utmp.h bits/utmp.h lastlog.h pty.h

routines := getutent getutent_r getutid getutline getutid_r getutline_r \
	    utmp_file utmp_daemon utmpname updwtmp \
	    getpt grantpt unlockpt ptsname

CFLAGS-grantpt.c = -DLIBEXECDIR='"$(libexecdir)"'

others = utmpd utmpdump pt_chown
install-sbin = utmpd
install-others = $(inst_libexecdir)/pt_chown
utmpd-routines := connection database error request xtmp
extra-objs := $(utmpd-routines:=.o)

distribute := utmp-private.h programs/xtmp.h programs/utmpd.h README.utmpd \
	      programs/utmpd-private.h $(utmpd-routines:%=programs/%.c) \
	      pty-internal.h

subdir-dirs = programs
vpath %.c programs

tests := tst-utmp tst-utmpx

# Build the -lutil library with these extra functions.
extra-libs      := libutil
extra-libs-others := $(extra-libs)

libutil-routines:= login login_tty logout logwtmp openpty forkpty

include ../Rules

# pt_chown needs to be setuid root.
$(inst_libexecdir)/pt_chown: $(objpfx)pt_chown $(+force)
	$(make-target-directory)
	-$(INSTALL) -m 4755 -o root $< $@

$(objpfx)utmpd: $(utmpd-routines:%=$(objpfx)%.o)

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libutil.so: $(common-objpfx)libc.so
