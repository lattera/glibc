# Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

test-srcs := collate-test xfrm-test tst-fmon
test-input := de_DE.ISO-8859-1 da_DK.ISO-8859-1 fr_CA,2.13.ISO-8859-1
test-output := $(foreach s, .out .xout, \
			 $(addsuffix $s, $(basename $(test-input))))
generated := $(test-input) $(test-output)
generated-dirs := $(basename $(test-input)) en_US

test-names := test1 test2 test3 test4
test-srcs := $(addprefix tests/,$(addsuffix .cm,$(test-names)) \
				$(addsuffix .def,$(test-names)))

distribute := CHECKSUMS tst-fmon.sh tst-fmon.data ChangeLog sort-test.sh \
	      README mnemonic.ds fr_CA,2.13.in de_DE.in da_DK.in	 \
	      $(charmaps) $(locales) $(test-srcs)

# Get $(inst_i18ndir) defined.
include ../Makeconfig

# Files to install.
install-others := $(addprefix $(inst_i18ndir)/, $(charmaps) $(locales))

include ../Rules

# Install the source files in the appropriate directories.
$(inst_i18ndir)/charmaps/%: charmaps/%; $(do-install)
$(inst_i18ndir)/locales/%: locales/%; $(do-install)


ifeq (no,$(cross-compiling))
tests: $(objpfx)collate-test $(objpfx)xfrm-test $(objpfx)tst-fmon
	$(SHELL) -e sort-test.sh $(common-objpfx) $(test-input)
	$(SHELL) -e tst-fmon.sh $(common-objpfx) tst-fmon.data
	$(SHELL) -e tst-locale.sh $(common-objpfx)
endif
