# Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
charmaps := $(filter-out $(addprefix charmaps/, CVS RCS SCCS %~), \
				     $(wildcard charmaps/[A-I]*) \
				     $(wildcard charmaps/[J-Z]*))

# List with all available character set descriptions.
locales := $(filter-out $(addprefix locales/, CVS RCS SCCS %~), \
				    $(wildcard locales/*))

# List of repertoire maps.
repertoiremaps := $(filter-out $(addprefix repertoiremaps/, CVS RCS SCCS %~), \
					   $(wildcard repertoiremaps/*))


subdir-dirs = tests-mbwc
vpath %.c tests-mbwc
vpath %.h tests-mbwc


test-srcs := collate-test xfrm-test tst-fmon tst-rpmatch tst-trans \
	     tst-mbswcs1 tst-mbswcs2 tst-mbswcs3 tst-mbswcs4 tst-mbswcs5 \
	     tst-ctype
#test-input := de_DE.ISO-8859-1 da_DK.ISO-8859-1 fr_CA,2.13.ISO-8859-1 \
#	      hr_HR.ISO-8859-2 # once it is fixed: cs_CZ.ISO-8859-2
test-input := de_DE.ISO-8859-1 en_US.ISO-8859-1
test-input-data = $(addsuffix .in, $(basename $(test-input)))
test-output := $(foreach s, .out .xout, \
			 $(addsuffix $s, $(basename $(test-input))))
ld-test-names := test1 test2 test3 test4 test5 test6
ld-test-srcs := $(addprefix tests/,$(addsuffix .cm,$(ld-test-names)) \
				   $(addsuffix .def,$(ld-test-names)) \
				   $(addsuffix .ds,test5 test6) \
				   test6.c trans.def)

fmon-tests = n01y12 n02n40 n10y31 n11y41 n12y11 n20n32 n30y20 n41n00 \
	     y01y10 y02n22 y22n42 y30y21 y32n31 y40y00 y42n21

generated := $(test-input) $(test-output)
generated-dirs := $(basename $(test-input)) $(ld-test-names) tt_TT	\
		  de_DE.437 $(addprefix tstfmon_,$(fmon-tests))		\
		  en_US.ANSI_X3.4-1968 ja_JP.EUC-JP

distribute := CHECKSUMS README SUPPORTED ChangeLog			\
	      $(charmaps) $(locales) $(repertoiremaps)			\
	      tst-rpmatch.sh tst-locale.sh tst-fmon.sh sort-test.sh	\
	      tst-fmon.data $(test-input-data) $(ld-test-srcs)		\
	      th_TH.in cs_CZ.in tst-mbswcs.sh tst-trans.sh tst-ctype.sh \
	      tst-ctype-de_DE.in

# Get $(inst_i18ndir) defined.
include ../Makeconfig

ifeq (no,$(cross-compiling))
ifeq (yes,$(build-shared))
# Disable the tests for now - first the locales have to be generated
locale_test_suite := tst_iswalnum tst_iswprint tst_towctrans tst_wcsncmp \
		     tst_wctrans tst_iswalpha tst_iswpunct tst_wcschr	 \
		     tst_wcspbrk tst_wctype tst_iswcntrl tst_iswspace	 \
		     tst_wcscoll tst_wcsspn tst_iswdigit tst_iswupper	 \
		     tst_wcscpy tst_wcsstr tst_iswgraph tst_iswxdigit	 \
		     tst_wcscspn tst_wcswidth tst_iswlower tst_swscanf	 \
		     tst_wcslen tst_wctob tst_iswctype tst_towlower	 \
		     tst_wcscat tst_towupper tst_wcscmp tst_wcsncat	 \
		     tst_wcsncpy tst_wcsxfrm tst_wcwidth tst_mbrlen
tests = $(locale_test_suite)
endif
endif

# Files to install.
install-others := $(addprefix $(inst_i18ndir)/, $(charmaps) $(locales) \
						$(repertoiremaps))

include ../Rules

# Install the source files in the appropriate directories.
$(inst_i18ndir)/charmaps/%: charmaps/% $(+force); $(do-install)
$(inst_i18ndir)/locales/%: locales/% $(+force); $(do-install)
$(inst_i18ndir)/repertoiremaps/%: repertoiremaps/% $(+force); $(do-install)

# gcc does not know all the format specifiers we are using here.
CFLAGS-tst-mbswcs1.c = -Wno-format
CFLAGS-tst-mbswcs2.c = -Wno-format
CFLAGS-tst-mbswcs3.c = -Wno-format
CFLAGS-tst-mbswcs4.c = -Wno-format
CFLAGS-tst-mbswcs5.c = -Wno-format
CFLAGS-tst-trans.c = -Wno-format


ifeq (no,$(cross-compiling))
ifeq (yes,$(build-shared))
.PHONY: do-collate-test do-tst-fmon do-tst-locale do-tst-rpmatch do-tst-trans \
	do-tst-mbswcs do-tst-ctype
tests: do-collate-test do-tst-fmon do-tst-locale do-tst-rpmatch do-tst-trans \
       do-tst-mbswcs do-tst-ctype
do-collate-test: sort-test.sh $(objpfx)collate-test $(objpfx)xfrm-test \
		 $(test-input-data)
	$(SHELL) -e $< $(common-objpfx) $(test-input)
do-tst-fmon: tst-fmon.sh $(objpfx)tst-fmon tst-fmon.data  do-collate-test
	$(SHELL) -e $< $(common-objpfx) tst-fmon.data
do-tst-locale: tst-locale.sh $(ld-test-srcs)
	$(SHELL) -e $< $(common-objpfx)
do-tst-rpmatch: tst-rpmatch.sh $(objpfx)tst-rpmatch do-tst-fmon
	$(SHELL) -e $< $(common-objpfx)
do-tst-trans: tst-trans.sh $(objpfx)tst-trans
	$(SHELL) -e $< $(common-objpfx)
do-tst-mbswcs: tst-mbswcs.sh $(objpfx)tst-mbswcs1 $(objpfx)tst-mbswcs2 \
	       $(objpfx)tst-mbswcs3 $(objpfx)tst-mbswcs4 $(objpfx)tst-mbswcs5
	$(SHELL) -e $< $(common-objpfx)
do-tst-ctype: tst-ctype.sh $(objpfx)tst-ctype do-collate-test do-tst-mbswcs
	$(SHELL) -e $< $(common-objpfx)
$(addsuffix .out,$(addprefix $(objpfx),$(locale_test_suite))): %: do-tst-ctype\
	do-tst-mbswcs
endif
endif

# Sometimes the whole collection of locale files should be installed.
LOCALEDEF=I18NPATH=. $(common-objpfx)elf/ld.so --library-path $(rpath-link) $(common-objpfx)locale/localedef
install-locales:
	$(..)./scripts/mkinstalldirs $(inst_localedir)
	while read locale charset; do \
	   case $$locale in \#*) continue;; esac; \
	   $(LOCALEDEF) -i locales/`echo $$locale | sed 's/\([^.]*\).*/\1/'` \
			-c -f charmaps/$$charset \
			-u repertoiremaps/mnemonic.ds \
			$(inst_localedir)/$$locale; \
	done < SUPPORTED

TEST_MBWC_ENV:= LOCPATH=$(common-objpfx)localedata
tst_iswalnum-ENV = $(TEST_MBWC_ENV)
tst_iswalpha-ENV = $(TEST_MBWC_ENV)
tst_iswcntrl-ENV = $(TEST_MBWC_ENV)
tst_iswctype-ENV = $(TEST_MBWC_ENV)
tst_iswdigit-ENV = $(TEST_MBWC_ENV)
tst_iswgraph-ENV = $(TEST_MBWC_ENV)
tst_iswlower-ENV = $(TEST_MBWC_ENV)
tst_iswprint-ENV = $(TEST_MBWC_ENV)
tst_iswpunct-ENV = $(TEST_MBWC_ENV)
tst_iswspace-ENV = $(TEST_MBWC_ENV)
tst_iswupper-ENV = $(TEST_MBWC_ENV)
tst_iswxdigit-ENV = $(TEST_MBWC_ENV)
tst_mbrlen-ENV = $(TEST_MBWC_ENV)
tst_swscanf-ENV = $(TEST_MBWC_ENV)
tst_towctrans-ENV = $(TEST_MBWC_ENV)
tst_towlower-ENV = $(TEST_MBWC_ENV)
tst_towupper-ENV = $(TEST_MBWC_ENV)
tst_wcscat-ENV = $(TEST_MBWC_ENV)
tst_wcschr-ENV = $(TEST_MBWC_ENV)
tst_wcscmp-ENV = $(TEST_MBWC_ENV)
tst_wcscoll-ENV = $(TEST_MBWC_ENV)
tst_wcscpy-ENV = $(TEST_MBWC_ENV)
tst_wcscspn-ENV = $(TEST_MBWC_ENV)
tst_wcslen-ENV = $(TEST_MBWC_ENV)
tst_wcsncat-ENV = $(TEST_MBWC_ENV)
tst_wcsncmp-ENV = $(TEST_MBWC_ENV)
tst_wcsncpy-ENV = $(TEST_MBWC_ENV)
tst_wcspbrk-ENV = $(TEST_MBWC_ENV)
tst_wcsspn-ENV = $(TEST_MBWC_ENV)
tst_wcsstr-ENV = $(TEST_MBWC_ENV)
tst_wcswidth-ENV = $(TEST_MBWC_ENV)
tst_wcsxfrm-ENV = $(TEST_MBWC_ENV)
tst_wctob-ENV = $(TEST_MBWC_ENV)
tst_wctrans-ENV = $(TEST_MBWC_ENV)
tst_wctype-ENV = $(TEST_MBWC_ENV)
tst_wcwidth-ENV = $(TEST_MBWC_ENV)
