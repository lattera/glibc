# Copyright (C) 1992, 1993, 1994, 1996, 1997 Free Software Foundation, Inc.
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

ifeq ($(subdir),termios)

ifeq (,$(filter termio.h,$(sysdep_headers)))

sysdep_headers := $(sysdep_headers) termio.h
generated := $(generated) termio.h

# termio.h is just like sysv_termio.h except it uses the same names for
# everything that System V termio does.  sysv_termio.h is necessary to
# include in __tcgetatr.c et al, because some of the names in termio.h
# conflict with termios.h.  The C library doesn't actually use termio.h,
# but we generate it for those application programs which use it.
$(objpfx)termio.h: $(..)sysdeps/unix/sysv/sysv_termio.h
	sed < $< > $@-tmp			\
	    -e 's/_SYSV_//'			\
	    -e 's/^#define[ 	]*_T/#define T/'\
	    -e 's/__sysv_termio/termio/'
	mv $@-tmp $@

endif
endif

# In SYSV style archives the symbol table member has an empty name.
ar-symtab-name =
