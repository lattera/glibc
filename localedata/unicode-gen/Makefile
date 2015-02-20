# Copyright (C) 2015 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

# Makefile for generating and updating Unicode-extracted files.


# This Makefile is NOT used as part of the GNU libc build.  It needs
# to be run manually, within the source tree, at Unicode upgrades
# (change UNICODE_VERSION below), to update ../locales/i18n ctype
# information (part of the file is preserved, so don't wipe it all
# out), and ../charmaps/UTF-8.

# Use make all to generate the files used in the glibc build out of
# the original Unicode files; make check to verify that they are what
# we expect; make install to copy them to the location expected by the
# glibc build; and make clean to remove all generated files.

# We keep a local copy of the downloaded Unicode files, to avoid
# running afoul of the LGPL corresponding sources requirements, even
# though it's not clear that they are preferred over the generated
# files for making modifications.


UNICODE_VERSION = 7.0.0

PYTHON3 = python3
WGET = wget

DOWNLOADS = UnicodeData.txt DerivedCoreProperties.txt EastAsianWidth.txt
GENERATED = i18n UTF-8
REPORTS = i18n-report UTF-8-report

all: $(GENERATED)

check: check-i18n check-UTF-8

install:
	cp -p i18n ../locales/i18n
	cp -p UTF-8 ../charmaps/UTF-8

clean: mostlyclean
	-rm -rf __pycache__
mostlyclean:
	-rm -f $(REPORTS) $(GENERATED)

.PHONY: all check clean mostlyclean install

i18n: UnicodeData.txt DerivedCoreProperties.txt
i18n: ../locales/i18n # Preserve non-ctype information.
i18n: gen_unicode_ctype.py
	$(PYTHON3) gen_unicode_ctype.py -u UnicodeData.txt \
	  -d DerivedCoreProperties.txt -i ../locales/i18n -o $@ \
	  --unicode_version $(UNICODE_VERSION)

i18n-report: i18n ../locales/i18n
i18n-report: ctype_compatibility.py ctype_compatibility_test_cases.py
	$(PYTHON3) ./ctype_compatibility.py -o ../locales/i18n \
	  -n i18n -a -m > $@

check-i18n: i18n-report
	@if grep '\(Missing\|Added\) [^0]\|^Number of errors[^=]* = [^0]' \
		i18n-report; \
	then echo manual verification required; false; else true; fi

UTF-8: UnicodeData.txt EastAsianWidth.txt
UTF-8: utf8_gen.py
	$(PYTHON3) utf8_gen.py UnicodeData.txt EastAsianWidth.txt

UTF-8-report: UTF-8 ../charmaps/UTF-8
UTF-8-report: utf8_compatibility.py
	$(PYTHON3) ./utf8_compatibility.py -o ../charmaps/UTF-8 \
	  -n UTF-8 -a -m > $@

check-UTF-8: UTF-8-report
	@if grep '^Total.*: [^0]' UTF-8-report; \
	then echo manual verification required; false; else true; fi


.PHONY: downloads clean-downloads
downloads: $(DOWNLOADS)
clean-downloads:
	-rm -f $(DOWNLOADS)

$(DOWNLOADS):
	$(WGET) http://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/$@
