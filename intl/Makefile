# Makefile for intl subdirectory: message handling code from GNU gettext.

# Copyright (C) 1995 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

subdir = intl
headers = libintl.h
routines = bindtextdom dcgettext dgettext gettext	\
	   finddomain loadmsgcat localealias textdomain
distribute = gettext.h gettextP.h hash-string.h

include ../Rules

CPPFLAGS += -D'GNULOCALEDIR="$(localedir)"' \
	    -D'LOCALE_ALIAS_PATH="$(localedir):$(nlsdir)"'

ifdef gettext-srcdir

%:: $(gettext-srcdir)/intl/%.glibc; $(copysrc)
%:: $(gettext-srcdir)/intl/%.c; $(copysrc)
%:: $(gettext-srcdir)/intl/%.h; $(copysrc)

define copysrc
cp -f $< $@.new
chmod a-w $@.new
mv -f $@.new $@
test ! -d CVS || cvs commit -m'Updated from $<' $@
endef

endif
