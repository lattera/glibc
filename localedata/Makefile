# Copyright (C) 1996-2003,2005,2007-2009,2011,2012
# Free Software Foundation, Inc.
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

# Makefile for installing locale data source files.

subdir := localedata

all: # Make this the default target; it will be defined in Rules.

# List with all available character set descriptions.
charmaps := $(wildcard charmaps/[A-I]*) $(wildcard charmaps/[J-Z]*)

# List with all available character set descriptions.
locales := $(wildcard locales/*)


subdir-dirs = tests-mbwc
vpath %.c tests-mbwc
vpath %.h tests-mbwc


test-srcs := collate-test xfrm-test tst-fmon tst-rpmatch tst-trans \
	     tst-mbswcs1 tst-mbswcs2 tst-mbswcs3 tst-mbswcs4 tst-mbswcs5 \
	     tst-ctype tst-wctype tst-langinfo tst-numeric
test-input := de_DE.ISO-8859-1 en_US.ISO-8859-1 da_DK.ISO-8859-1 \
	      hr_HR.ISO-8859-2 sv_SE.ISO-8859-1 tr_TR.UTF-8 fr_FR.UTF-8 \
	      si_LK.UTF-8
test-input-data = $(addsuffix .in, $(basename $(test-input)))
test-output := $(foreach s, .out .xout, \
			 $(addsuffix $s, $(basename $(test-input))))
ld-test-names := test1 test2 test3 test4 test5 test6 test7
ld-test-srcs := $(addprefix tests/,$(addsuffix .cm,$(ld-test-names)) \
				   $(addsuffix .def,$(ld-test-names)) \
				   $(addsuffix .ds,test5 test6) \
				   test6.c trans.def)

fmon-tests = n01y12 n02n40 n10y31 n11y41 n12y11 n20n32 n30y20 n41n00 \
	     y01y10 y02n22 y22n42 y30y21 y32n31 y40y00 y42n21

generated := $(test-input) $(test-output) sort-test.out tst-locale.out \
	     tst-mbswcs.out tst-leaks.mtrace mtrace-tst-leaks
generated-dirs := $(ld-test-names) tt_TT de_DE.437			\
		  $(addprefix tstfmon_,$(fmon-tests))			\

# Get $(inst_i18ndir) defined.
include ../Makeconfig

ifeq (no,$(cross-compiling))
locale_test_suite := tst_iswalnum tst_iswalpha tst_iswcntrl            \
		     tst_iswctype tst_iswdigit tst_iswgraph            \
		     tst_iswlower tst_iswprint tst_iswpunct            \
		     tst_iswspace tst_iswupper tst_iswxdigit tst_mblen \
		     tst_mbrlen tst_mbrtowc tst_mbsrtowcs tst_mbstowcs \
		     tst_mbtowc tst_strcoll tst_strfmon tst_strxfrm    \
		     tst_swscanf tst_towctrans tst_towlower            \
		     tst_towupper tst_wcrtomb tst_wcscat tst_wcschr    \
		     tst_wcscmp tst_wcscoll tst_wcscpy tst_wcscspn     \
		     tst_wcslen tst_wcsncat tst_wcsncmp tst_wcsncpy    \
		     tst_wcspbrk tst_wcsrtombs tst_wcsspn tst_wcsstr   \
		     tst_wcstod tst_wcstok tst_wcstombs tst_wcswidth   \
		     tst_wcsxfrm tst_wctob tst_wctomb tst_wctrans      \
		     tst_wctype tst_wcwidth

tests = $(locale_test_suite) tst-digits tst-setlocale bug-iconv-trans \
	tst-leaks tst-mbswcs6 tst-xlocale1 tst-xlocale2 bug-usesetlocale \
	tst-strfmon1 tst-sscanf tst-strptime bug-setlocale1 tst-setlocale2
ifeq (yes,$(build-shared))
ifneq (no,$(PERL))
tests: $(objpfx)mtrace-tst-leaks
endif
endif
endif

# Files to install.
install-others := $(addprefix $(inst_i18ndir)/, \
			      $(addsuffix .gz, $(charmaps)) \
			      $(locales))

tests: $(objdir)/iconvdata/gconv-modules

include ../Rules

# Install the charmap files in gzipped format.
$(inst_i18ndir)/charmaps/%.gz: charmaps/% $(+force)
	$(make-target-directory)
	rm -f $(@:.gz=) $@
	$(INSTALL_DATA) $< $(@:.gz=)
	gzip -9n $(@:.gz=)

# Install the locale source files in the appropriate directory.
$(inst_i18ndir)/locales/%: locales/% $(+force); $(do-install)

# gcc does not know all the format specifiers we are using here.
CFLAGS-tst-mbswcs1.c = -Wno-format
CFLAGS-tst-mbswcs2.c = -Wno-format
CFLAGS-tst-mbswcs3.c = -Wno-format
CFLAGS-tst-mbswcs4.c = -Wno-format
CFLAGS-tst-mbswcs5.c = -Wno-format
CFLAGS-tst-trans.c = -Wno-format


ifeq (no,$(cross-compiling))
# We have to generate locales
LOCALES := de_DE.ISO-8859-1 de_DE.UTF-8 en_US.ANSI_X3.4-1968 \
	   en_US.ISO-8859-1 ja_JP.EUC-JP da_DK.ISO-8859-1 \
	   hr_HR.ISO-8859-2 sv_SE.ISO-8859-1 ja_JP.SJIS fr_FR.ISO-8859-1 \
	   vi_VN.TCVN5712-1 nb_NO.ISO-8859-1 nn_NO.ISO-8859-1 \
	   tr_TR.UTF-8 cs_CZ.UTF-8 zh_TW.EUC-TW fa_IR.UTF-8 fr_FR.UTF-8 \
	   ja_JP.UTF-8 si_LK.UTF-8 tr_TR.ISO-8859-9
LOCALE_SRCS := $(shell echo "$(LOCALES)"|sed 's/\([^ .]*\)[^ ]*/\1/g')
CHARMAPS := $(shell echo "$(LOCALES)" | \
		    sed -e 's/[^ .]*[.]\([^ ]*\)/\1/g' -e s/SJIS/SHIFT_JIS/g)
CTYPE_FILES = $(addsuffix /LC_CTYPE,$(LOCALES))

generated-dirs += $(LOCALES)

# Dependency for the locale files.  We actually make it depend only on
# one of the files.
$(addprefix $(objpfx),$(CTYPE_FILES)): %: \
  gen-locale.sh $(common-objpfx)locale/localedef Makefile \
  $(addprefix charmaps/,$(CHARMAPS)) $(addprefix locales/,$(LOCALE_SRCS))
	@$(SHELL) -e gen-locale.sh $(common-objpfx) '$(built-program-cmd)' $@

$(addsuffix .out,$(addprefix $(objpfx),$(tests))): %: \
  $(addprefix $(objpfx),$(CTYPE_FILES))

tests: $(objpfx)sort-test.out $(objpfx)tst-fmon.out $(objpfx)tst-locale.out \
       $(objpfx)tst-rpmatch.out $(objpfx)tst-trans.out \
       $(objpfx)tst-mbswcs.out $(objpfx)tst-ctype.out $(objpfx)tst-wctype.out \
       $(objpfx)tst-langinfo.out $(objpfx)tst-numeric.out

$(objpfx)sort-test.out: sort-test.sh $(objpfx)collate-test $(objpfx)xfrm-test \
		       $(test-input-data) $(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(run-program-prefix)' $(test-input) \
	  > $@
$(objpfx)tst-fmon.out: tst-fmon.sh $(objpfx)tst-fmon tst-fmon.data \
		       $(objpfx)sort-test.out \
		       $(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(run-program-prefix)' tst-fmon.data \
	  > $@
$(objpfx)tst-numeric.out: tst-numeric.sh $(objpfx)tst-numeric tst-numeric.data \
		       $(objpfx)sort-test.out \
		       $(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(run-program-prefix)' tst-numeric.data \
	  > $@
$(objpfx)tst-locale.out: tst-locale.sh $(common-objpfx)locale/localedef \
			 $(ld-test-srcs) $(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(built-program-cmd)' > $@
$(objpfx)tst-rpmatch.out: tst-rpmatch.sh $(objpfx)tst-rpmatch \
			  $(objpfx)tst-fmon.out \
			  $(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(built-program-cmd)' > $@
$(objpfx)tst-trans.out: tst-trans.sh $(objpfx)tst-trans \
			$(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(run-program-prefix)'
$(objpfx)tst-mbswcs.out: tst-mbswcs.sh $(objpfx)tst-mbswcs1 \
			 $(objpfx)tst-mbswcs2 $(objpfx)tst-mbswcs3 \
			 $(objpfx)tst-mbswcs4 $(objpfx)tst-mbswcs5 \
			 $(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(run-program-prefix)'
$(objpfx)tst-ctype.out: tst-ctype.sh $(objpfx)tst-ctype \
			$(objpfx)sort-test.out \
			$(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(built-program-cmd)'
$(objpfx)tst-wctype.out: tst-wctype.sh $(objpfx)tst-wctype \
			$(objpfx)sort-test.out tst-wctype.input \
			$(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(built-program-cmd)'
$(objpfx)tst-langinfo.out: tst-langinfo.sh $(objpfx)tst-langinfo \
			$(objpfx)sort-test.out \
			$(addprefix $(objpfx),$(CTYPE_FILES))
	$(SHELL) -e $< $(common-objpfx) '$(built-program-cmd)'
$(objpfx)tst-digits.out: $(objpfx)tst-locale.out
$(objpfx)tst-mbswcs6.out: $(addprefix $(objpfx),$(CTYPE_FILES))
endif

include SUPPORTED

INSTALL-SUPPORTED-LOCALES=$(addprefix install-, $(SUPPORTED-LOCALES))

# Sometimes the whole collection of locale files should be installed.
LOCALEDEF=I18NPATH=. GCONV_PATH=$(common-objpfx)iconvdata LC_ALL=C \
$(common-objpfx)elf/ld.so --library-path $(rpath-link) $(common-objpfx)locale/localedef
install-locales: $(INSTALL-SUPPORTED-LOCALES)

install-locales-dir:
	$(..)./scripts/mkinstalldirs $(inst_localedir)

$(INSTALL-SUPPORTED-LOCALES): install-locales-dir
	@locale=`echo $@ | sed -e 's/^install-//'`; \
	charset=`echo $$locale | sed -e 's,.*/,,'`; \
	locale=`echo $$locale | sed -e 's,/[^/]*,,'`; \
	echo -n `echo $$locale | sed 's/\([^.\@]*\).*/\1/'`; \
	echo -n ".$$charset"; \
	echo -n `echo $$locale | sed 's/\([^\@]*\)\(\@.*\)*/\2/'`; \
	echo -n '...'; \
	input=`echo $$locale | sed 's/\([^.]*\)[^@]*\(.*\)/\1\2/'`; \
	$(LOCALEDEF) --alias-file=../intl/locale.alias \
		     -i locales/$$input -c -f charmaps/$$charset \
		     $(addprefix --prefix=,$(install_root)) $$locale; \
	echo ' done'; \

# The mbwc-tests need some environment setup to find the locale data files
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
tst_mblen-ENV = $(TEST_MBWC_ENV)
tst_mbrlen-ENV = $(TEST_MBWC_ENV)
tst_mbrtowc-ENV = $(TEST_MBWC_ENV)
tst_mbsrtowcs-ENV = $(TEST_MBWC_ENV)
tst_mbstowcs-ENV = $(TEST_MBWC_ENV)
tst_mbtowc-ENV = $(TEST_MBWC_ENV)
tst_strcoll-ENV = $(TEST_MBWC_ENV)
tst_strfmon-ENV = $(TEST_MBWC_ENV)
tst_strxfrm-ENV = $(TEST_MBWC_ENV)
tst_swscanf-ENV = $(TEST_MBWC_ENV)
tst_towctrans-ENV = $(TEST_MBWC_ENV)
tst_towlower-ENV = $(TEST_MBWC_ENV)
tst_towupper-ENV = $(TEST_MBWC_ENV)
tst_wcrtomb-ENV = $(TEST_MBWC_ENV)
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
tst_wcsrtombs-ENV = $(TEST_MBWC_ENV)
tst_wcsspn-ENV = $(TEST_MBWC_ENV)
tst_wcsstr-ENV = $(TEST_MBWC_ENV)
tst_wcstod-ENV = $(TEST_MBWC_ENV)
tst_wcstok-ENV = $(TEST_MBWC_ENV)
tst_wcstombs-ENV = $(TEST_MBWC_ENV)
tst_wcswidth-ENV = $(TEST_MBWC_ENV)
tst_wcsxfrm-ENV = $(TEST_MBWC_ENV)
tst_wctob-ENV = $(TEST_MBWC_ENV)
tst_wctomb-ENV = $(TEST_MBWC_ENV)
tst_wctrans-ENV = $(TEST_MBWC_ENV)
tst_wctype-ENV = $(TEST_MBWC_ENV)
tst_wcwidth-ENV = $(TEST_MBWC_ENV)
tst-digits-ENV = $(TEST_MBWC_ENV)
tst-mbswcs6-ENV = $(TEST_MBWC_ENV)
tst-xlocale1-ENV = $(TEST_MBWC_ENV)
tst-xlocale2-ENV = $(TEST_MBWC_ENV)
tst-strfmon1-ENV = $(TEST_MBWC_ENV)
tst-strptime-ENV = $(TEST_MBWC_ENV)

tst-setlocale-ENV = LOCPATH=$(common-objpfx)localedata LC_ALL=ja_JP.EUC-JP

bug-iconv-trans-ENV = LOCPATH=$(common-objpfx)localedata

tst-sscanf-ENV = LOCPATH=$(common-objpfx)localedata

tst-leaks-ENV = MALLOC_TRACE=$(objpfx)tst-leaks.mtrace \
		LOCPATH=$(common-objpfx)localedata
$(objpfx)mtrace-tst-leaks: $(objpfx)tst-leaks.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-leaks.mtrace > $@

bug-setlocale1-ENV = LOCPATH=$(common-objpfx)localedata
bug-setlocale1-ARGS = $(common-objpfx)
tst-setlocale2-ENV = LOCPATH=$(common-objpfx)localedata

$(objdir)/iconvdata/gconv-modules:
	$(MAKE) -C ../iconvdata subdir=iconvdata $@
