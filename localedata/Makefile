# Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

# Makefile for installing locale data source files.

subdir := localedata

all: # Make this the default target; it will be defined in Rules.

# List with all available character set descriptions.
charmaps := $(filter-out $(addprefix charmaps/, CVS RCS %~), \
				     $(wildcard charmaps/*))

# List with all available character set descriptions.
locales := $(filter-out $(addprefix locales/, CVS RCS %~), \
				    $(wildcard locales/*))

test-srcs := collate-test xfrm-test
test-input := de_DE.ISO-8859-1 da_DK.ISO-8859-1 fr_CA,2.13.ISO-8859-1


# Get $(inst_i18ndir) defined.
include ../Makeconfig

# Files to install.
install-others := $(addprefix $(inst_i18ndir)/, $(charmaps) $(locales))

include ../Rules

# Install the message object files as libc.po in the language directory.
$(inst_i18ndir)/charmaps/%: charmaps/%; $(do-install)
$(inst_i18ndir)/locales/%: locales/%; $(do-install)


ifeq (no,$(cross-compiling))
tests: $(objpfx)collate-test $(objpfx)xfrm-test
	$(SHELL) -e sort-test.sh $(common-objpfx) $(test-input)
endif
