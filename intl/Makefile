# Copyright (C) 1995-1999, 2000 Free Software Foundation, Inc.
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
	   dcigettext dcngettext dngettext ngettext \
	   finddomain loadmsgcat localealias textdomain	\
	   l10nflist explodename plural
distribute = gettext.h gettextP.h hash-string.h loadinfo.h locale.alias \
	     plural.y po2test.sed tst-gettext.sh

test-srcs := tst-gettext

before-compile = $(objpfx)msgs.h

install-others = $(inst_msgcatdir)/locale.alias

generated = msgs.h
generated-dirs := domaindir

plural.c: plural.y
	$(YACC) $(YFLAGS) $@ $^
ifeq ($(with-cvs),yes)
	test ! -d CVS || cvs $(CVSOPTS) commit -m'$(YACC) $(YFLAGS) $@ $^' $@
endif
$(objpfx)plural.o: plural.c

include ../Rules

ifeq (no,$(cross-compiling))
ifeq (yes,$(build-shared))
ifneq ($(strip $(MSGFMT)),:)
.PHONY: do-gettext-test
tests: do-gettext-test
do-gettext-test: tst-gettext.sh $(objpfx)tst-gettext
	$(SHELL) -e $< $(common-objpfx) $(common-objpfx)intl/
endif
endif
endif

$(objpfx)msgs.h: po2test.sed ../po/de.po
	$(make-target-directory)
	sed -f $^ > $@

CFLAGS-tst-gettext.c = -DTESTSTRS_H=\"$(objpfx)msgs.h\"

CPPFLAGS += -D'GNULOCALEDIR="$(msgcatdir)"' \
	    -D'LOCALE_ALIAS_PATH="$(msgcatdir):$(i18ndir)"'
YFLAGS = --name-prefix=__gettext --output

$(inst_msgcatdir)/locale.alias: locale.alias $(+force)
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
