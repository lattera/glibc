# Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

# Makefile for intl subdirectory: message handling code from GNU gettext.

subdir = intl
headers = libintl.h
routines = bindtextdom dcgettext dgettext gettext	\
	   finddomain loadmsgcat localealias textdomain	\
	   l10nflist explodename
distribute = gettext.h gettextP.h hash-string.h loadinfo.h locale.alias

install-others = $(inst_localedir)/locale.alias

include ../Rules

CPPFLAGS += -D'GNULOCALEDIR="$(localedir)"' \
	    -D'LOCALE_ALIAS_PATH="$(localedir):$(i18ndir)"'

$(inst_localedir)/locale.alias: locale.alias $(+force)
	$(do-install)

ifdef gettext-srcdir

%.h:: ../gpl2lgpl.sed $(gettext-srcdir)/intl/%.glibc; $(copysrc)
%.c:: ../gpl2lgpl.sed $(gettext-srcdir)/intl/%.c; $(copysrc)
%.h:: ../gpl2lgpl.sed $(gettext-srcdir)/intl/%.h; $(copysrc)
locale.alias:: ../gpl2lgpl.sed $(gettext-srcdir)/misc/locale.alias; $(copysrc)

ifeq ($(with-cvs),yes)
define copysrc
sed -f $^ > $@.new
chmod a-w $@.new
mv -f $@.new $@
test ! -d CVS || cvs $(CVSOPTS) commit -m'Updated from $<' $@
endef
else
define copysrc
sed -f $^ > $@.new
chmod a-w $@.new
mv -f $@.new $@
endef
endif

endif

CFLAGS-bindtextdom.c = -Wno-unused
CFLAGS-finddomain.c = -Wno-unused
CFLAGS-localealias.c = -Wno-unused
