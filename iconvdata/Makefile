# Copyright (C) 1997, 1998 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#
#	Makefile for iconv data and code.
#
subdir	:= iconvdata

# Names of all the shared objects which implement the transformations.
modules	:= ISO8859-1 ISO8859-2 ISO8859-3 ISO8859-4 ISO8859-5		\
	   ISO8859-6 ISO8859-7 ISO8859-8 ISO8859-9 ISO8859-10		\
	   T.61 ISO_6937 SJIS KOI-8 KOI8-R LATIN-GREEK LATIN-GREEK-1	\
	   HP-ROMAN8 EBCDIC-AT-DE EBCDIC-AT-DE-A EBCDIC-CA-FR		\
	   EUC-KR UHC JOHAB libJIS libKSC ISO646 BIG5 EUC-JP libGB	\
	   EUC-CN libCNS EUC-TW
modules.so := $(addsuffix .so, $(modules))


include ../Makeconfig

ISO8859-1-routines := iso8859-1
ISO8859-2-routines := iso8859-2
ISO8859-3-routines := iso8859-3
ISO8859-4-routines := iso8859-4
ISO8859-5-routines := iso8859-5
ISO8859-6-routines := iso8859-6
ISO8859-7-routines := iso8859-7
ISO8859-8-routines := iso8859-8
ISO8859-9-routines := iso8859-9
ISO8859-10-routines := iso8859-10
ISO646-routines := iso646
T.61-routines := t61
ISO_6937-routines := iso6937
SJIS-routines := sjis
KOI-8-routines := koi-8
KOI8-R-routines := koi8-r
LATIN-GREEK-routines := latin-greek
LATIN-GREEK-1-routines := latin-greek-1
HP-ROMAN8-routines := hp-roman8
EBCDIC-AT-DE-routines := ebcdic-at-de
EBCDIC-AT-DE-A-routines := ebcdic-at-de-a
EBCDIC-CA-FR-routines := ebcdic-ca-fr
EUC-KR-routines := euckr
JOHAB-routines := johab
UHC-routines := uhc
BIG5-routines := big5
EUC-JP-routines := eucjp
EUC-CN-routines := euccn
EUC-TW-routines := euctw
libJIS-routines := jis0201 jis0208 jis0212
libKSC-routines := ksc5601
libGB-routines := gb2312
libCNS-routines := cns11643l1 cns11643

LDFLAGS-EUC-KR.so = -Wl,-rpath,$(gconvdir)
$(objpfx)EUC-KR.so: $(objpfx)libKSC.so
LDFLAGS-JOHAB.so = -Wl,-rpath,$(gconvdir)
$(objpfx)JOHAB.so: $(objpfx)libKSC.so
LDFLAGS-UHC.so = -Wl,-rpath,$(gconvdir)
$(objpfx)UHC.so: $(objpfx)libKSC.so
LDFLAGS-EUC-JP.so = -Wl,-rpath,$(gconvdir)
$(objpfx)EUC-JP.so: $(objpfx)libJIS.so
LDFLAGS-EUC-CN.so = -Wl,-rpath,$(gconvdir)
$(objpfx)EUC-CN.so: $(objpfx)libGB.so
LDFLAGS-EUC-TW.so = -Wl,-rpath,$(gconvdir)
$(objpfx)EUC-TW.so: $(objpfx)libCNS.so

LDFLAGS-libJIS.so = -Wl,-soname,$(@F)
LDFLAGS-libKSC.so = -Wl,-soname,$(@F)
LDFLAGS-libGB.so = -Wl,-soname,$(@F)
LDFLAGS-libCNS.so = -Wl,-soname,$(@F)

distribute := 8bit-generic.c 8bit-gap.c gap.pl gaptab.pl gconv-modules	    \
	      iso8859-1.c iso8859-2.c iso8859-3.c iso8859-4.c iso8859-5.c   \
	      iso8859-6.c iso8859-7.c iso8859-8.c iso8859-9.c iso8859-10.c  \
	      t61.c iso6937.c sjis.c jis0201.h jis0208.h jis0212.h	    \
	      koi-8.c koi8-r.c koi8-r.h hp-roman8.c latin-greek.c	    \
	      latin-greek.h latin-greek-1.c latin-greek-1.h ebcdic-at-de.c  \
	      ebcdic-at-de-a.c ebcdic-ca-fr.c jis0201.c jis0208.c jis0212.c \
	      extra-module.mk euckr.c johab.c uhc.c ksc5601.c ksc5601.h	    \
	      iso646.c big5.c eucjp.c gb2312.c gb2312.h euccn.c euctw.c	    \
	      cns11643l1.c cns11643l1.h cns11643.h cns11643.c

# We build the transformation modules only when we build shared libs.
ifeq (yes,$(build-shared))

# This macro is similar to build-shlib but it does not define a soname
# and it does not depend on the destination name to start with `lib'.
define build-module
$(LINK.o) -shared -o $@ $(sysdep-LDFLAGS) $(config-LDFLAGS)  \
	  -B$(csu-objpfx) $(load-map-file:%=-Wl,--version-script=%) \
	  $(LDFLAGS.so) $(LDFLAGS-$(@F:%.so=%).so) \
	  -L$(subst :, -L,$(rpath-link)) -Wl,-rpath-link=$(rpath-link) \
	  -Wl,--whole-archive \
	  $(filter-out $($(@F:.so=)-map) $(+preinit) $(+postinit),$^) \
	  $(no-whole-archive) $(LDLIBS-$(@F:%.so=%).so)
endef

# Rule to generate the shared objects.
extra-modules-left := $(modules)
include extra-module.mk


extra-objs	+= $(modules.so)
install-others	= $(addprefix $(inst_gconvdir)/, $(modules.so))	\
		  $(inst_gconvdir)/gconv-modules

# If we have the localedata add-on available we can build the conversion
# tables for numerous charsets.
define generate-8bit-table
( echo "static const wchar_t to_ucs4[256] = {"; \
  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\1] = 0x\2,/p' -e d $^ | sort -u; \
  echo "};"; \
  echo "static const char from_ucs4[] = {"; \
  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\2] = 0x\1,/p' -e d $^ | sort -u; \
  echo "};" ) > $@.new && rm -f $@ && mv $@.new $@
endef

sed-generated-headers := iso8859-2.h iso8859-3.h iso8859-4.h iso8859-5.h \
			 iso8859-6.h iso8859-7.h iso8859-8.h iso8859-9.h \
			 iso8859-10.h koi-8.h hp-roman8.h ebcdic-at-de.h \
			 ebcdic-at-de-a.h ebcdic-ca-fr.h

define generate-8bit-gap-table
( echo "static const wchar_t to_ucs4[256] = {"; \
  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\1] = 0x\2,/p' -e d $^ | sort -u; \
  echo "};"; \
  echo "static struct gap from_idx[] = {"; \
  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/0x\2 0x\1/p' -e d $^ | sort -u | $(PERL) gap.pl; \
  echo "  { start: 0xffff, end: 0xffff, idx:     0 }"; \
  echo "};"; \
  echo "static const char from_ucs4[] = {"; \
  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/0x\2 0x\1/p' -e d $^ | sort -u | $(PERL) gaptab.pl; \
  echo "};" ) > $@.new && rm -f $@ && mv $@.new $@
endef

perl-generated-headers := koi8-r.h latin-greek.h latin-greek-1.h

# The headers must be generated before the compilation.
before-compile = $(sed-generated-headers) $(perl-generated-headers)


# Rules to generate the headers.
iso8859-2.h: ../localedata/charmaps/ISO-8859-2 Makefile
	$(generate-8bit-table)
iso8859-3.h: ../localedata/charmaps/ISO-8859-3 Makefile
	$(generate-8bit-table)
iso8859-4.h: ../localedata/charmaps/ISO-8859-4 Makefile
	$(generate-8bit-table)
iso8859-5.h: ../localedata/charmaps/ISO-8859-5 Makefile
	$(generate-8bit-table)
iso8859-6.h: ../localedata/charmaps/ISO-8859-6 Makefile
	$(generate-8bit-table)
iso8859-7.h: ../localedata/charmaps/ISO-8859-7 Makefile
	$(generate-8bit-table)
iso8859-8.h: ../localedata/charmaps/ISO-8859-8 Makefile
	$(generate-8bit-table)
iso8859-9.h: ../localedata/charmaps/ISO-8859-9 Makefile
	$(generate-8bit-table)
iso8859-10.h: ../localedata/charmaps/ISO-8859-10 Makefile
	$(generate-8bit-table)

koi-8.h: ../localedata/charmaps/KOI-8 Makefile
	$(generate-8bit-table)

hp-roman8.h: ../localedata/charmaps/HP-ROMAN8 Makefile
	$(generate-8bit-table)

ebcdic-at-de.h: ../localedata/charmaps/EBCDIC-AT-DE Makefile
	$(generate-8bit-table)
ebcdic-at-de-a.h: ../localedata/charmaps/EBCDIC-AT-DE-A Makefile
	$(generate-8bit-table)
ebcdic-ca-fr.h: ../localedata/charmaps/EBCDIC-CA-FR Makefile
	$(generate-8bit-table)

ifneq ($(PERL),no)
koi8-r.h: ../localedata/charmaps/KOI8-R Makefile
	$(generate-8bit-gap-table)
latin-greek.h: ../localedata/charmaps/LATIN-GREEK Makefile
	$(generate-8bit-gap-table)
latin-greek-1.h: ../localedata/charmaps/LATIN-GREEK-1 Makefile
	$(generate-8bit-gap-table)
endif

headers: $(sed-generated-headers) $(perl-generated-headers)


$(addprefix $(inst_gconvdir)/, $(modules.so)): \
    $(inst_gconvdir)/%: $(objpfx)% $(+force)
	$(do-install-program)
$(inst_gconvdir)/gconv-modules: gconv-modules $(+force)
	$(do-install)
endif

include ../Rules
