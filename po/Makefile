# Copyright (C) 1996,1997,1998,1999,2001,2002 Free Software Foundation, Inc.
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

# Makefile for installing libc message catalogs.

subdir := po

# Add names of the languages with broken .po files here.
BROKEN_LINGUAS = 

# List of languages for which we have message catalogs of translations.
ALL_LINGUAS := $(filter-out $(BROKEN_LINGUAS),$(basename $(wildcard *.po)))

# You can override this in configparms or the make command line to limit
# the languages which get installed.
ifdef LINGUAS
LINGUAS := $(filter $(addsuffix %,$(LINGUAS)),$(ALL_LINGUAS))
else
LINGUAS = $(ALL_LINGUAS)
endif

# Text domain name to install under; must match ../locale/SYS_libc.c string.
domainname = libc

all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

# Pattern for where message catalog object for language % gets installed.
mo-installed = $(inst_msgcatdir)/%/LC_MESSAGES/$(domainname).mo

# Files to install: a $(domainname).mo file for each language.
install-others = $(LINGUAS:%=$(mo-installed))

# Files to distribute: all the source and compiled binary translation files.
distribute = $(ALL_LINGUAS:=.po) $(BROKEN_LINGUAS:=.po) $(ALL_LINGUAS:=.mo) \
	     libc.pot header.pot


include ../Rules


.SUFFIXES: .mo .po .pot

# Compile the binary message object files from the portable object source
# files of translations for each language.
%.mo: %.po
	$(MSGFMT) -o $@ $<

# Install the message object files as libc.po in the language directory.
$(mo-installed): %.mo $(+force); $(do-install) || exit 0

.PHONY: linguas linguas.mo
linguas: $(ALL_LINGUAS:=.po)
linguas.mo: $(ALL_LINGUAS:=.mo)

realclean:
	rm -f $(ALL_LINGUAS:=.mo)

# Copy the PO files from the translation coordinator's repository.

podir = /com/share/ftp/gnu/po/maint/glibc

pofiles := $(wildcard $(podir)/*.po)

ifneq (,$(pofiles))

%.po: $(podir)/%.po
	cp -f $< $@
	chmod 444 $@

linguas: $(pofiles:$(podir)/%=%)
ifeq ($(with-cvs),yes)
	test ! -d CVS || cvs $(CVSOPTS) commit -m'Copied from $(podir)' $^
endif
linguas.mo: $(pofiles:$(podir)/%.po=%.mo)

endif
