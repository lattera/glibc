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
modules	:= ISO8859-1 ISO8859-2 ISO8859-3 ISO8859-4 ISO8859-5		 \
	   ISO8859-6 ISO8859-7 ISO8859-8 ISO8859-9 ISO8859-10		 \
	   T.61 ISO_6937 SJIS KOI-8 HP-ROMAN8 EBCDIC-AT-DE		 \
	   EBCDIC-AT-DE-A EBCDIC-CA-FR EUC-KR UHC JOHAB libJIS libKSC	 \
	   BIG5 EUC-JP libGB EUC-CN libCNS EUC-TW ISO646 EBCDIC-DK-NO	 \
	   EBCDIC-DK-NO-A EBCDIC-ES EBCDIC-ES-A EBCDIC-ES-S EBCDIC-FI-SE \
	   EBCDIC-FI-SE-A EBCDIC-FR EBCDIC-IS-FRISS EBCDIC-IT EBCDIC-PT	 \
	   EBCDIC-UK EBCDIC-US IBM037 IBM038 IBM274 IBM275 IBM423 IBM424 \
	   IBM500 IBM870 IBM871 IBM891 IBM903 IBM904 IBM905 IBM1047	 \
	   CP874 CP737 CP775 ISO-2022-KR
ifneq ($(PERL),no)
modules += KOI8-R LATIN-GREEK LATIN-GREEK-1 IBM256 IBM273 IBM277 IBM278	 \
	   IBM280 IBM281 IBM284 IBM285 IBM290 IBM297 IBM420 IBM437	 \
	   IBM850 IBM851 IBM852 IBM855 IBM857 IBM860 IBM861 IBM862	 \
	   IBM863 IBM864 IBM865 IBM868 IBM869 IBM875 IBM880 IBM918	 \
	   IBM1004 IBM1026 CP1250 CP1251 CP1252 CP1253 CP1254 CP1255	 \
	   CP1256 CP1257 ISO-2022-JP
endif
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
EBCDIC-DK-NO-routines := ebcdic-dk-no
EBCDIC-DK-NO-A_routines := ebcdic-dk-no-a
EBCDIC-ES-routines := ebcdic-es
EBCDIC-ES-A-routines := ebcdic-es-a
EBCDIC-ES-S-routines := ebcdic-es-s
EBCDIC-FI-SE-routines := ebcdic-fi-se
EBCDIC-FI-SE-A-routines := ebcdic-fi-se-a
EBCDIC-FR-routines := ebcdic-fr
EBCDIC-IS-FRISS-routines := ebcdic-is-friss
EBCDIC-IT-routines := ebcdic-it
EBCDIC-PT-routines := ebcdic-pt
EBCDIC-UK-routines := ebcdic-uk
EBCDIC-US-routines := ebcdic-us
IBM037-routines := ibm037
IBM038-routines := ibm038
IBM256-routines := ibm256
IBM273-routines := ibm273
IBM274-routines := ibm274
IBM275-routines := ibm275
IBM277-routines := ibm277
IBM278-routines := ibm278
IBM280-routines := ibm280
IBM281-routines := ibm281
IBM284-routines := ibm284
IBM285-routines := ibm285
IBM290-routines := ibm290
IBM297-routines := ibm297
IBM420-routines := ibm420
IBM423-routines := ibm423
IBM424-routines := ibm424
IBM437-routines := ibm437
IBM500-routines := ibm500
IBM850-routines := ibm850
IBM851-routines := ibm851
IBM852-routines := ibm852
IBM855-routines := ibm855
IBM857-routines := ibm857
IBM860-routines := ibm860
IBM861-routines := ibm861
IBM862-routines := ibm862
IBM863-routines := ibm863
IBM864-routines := ibm864
IBM865-routines := ibm865
IBM868-routines := ibm868
IBM869-routines := ibm869
IBM870-routines := ibm870
IBM871-routines := ibm871
IBM875-routines := ibm875
IBM880-routines := ibm880
IBM891-routines := ibm891
IBM903-routines := ibm903
IBM904-routines := ibm904
IBM905-routines := ibm905
IBM918-routines := ibm918
IBM1004-routines := ibm1004
IBM1026-routines := ibm1026
IBM1047-routines := ibm1047
CP737-routines := cp737
CP775-routines := cp775
CP874-routines := cp874
CP1250-routines := cp1250
CP1251-routines := cp1251
CP1252-routines := cp1252
CP1253-routines := cp1253
CP1254-routines := cp1254
CP1255-routines := cp1255
CP1256-routines := cp1256
CP1257-routines := cp1257
EUC-KR-routines := euckr
JOHAB-routines := johab
UHC-routines := uhc
BIG5-routines := big5
EUC-JP-routines := eucjp
EUC-CN-routines := euccn
EUC-TW-routines := euctw
ISO-2022-JP-routines := iso-2022-jp
ISO-2022-KR-routines := iso-2022-kr
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

LDFLAGS-ISO-2022-JP.so = -Wl,-rpath,$(gconvdir)
$(objpfx)ISO-2022-JP.so: $(objpfx)libJIS.so $(objpfx)libGB.so \
			 $(objpfx)libCNS.so $(objpfx)libKSC.so
LDFLAGS-ISO-2022-JP.so = -Wl,-rpath,$(gconvdir)
$(objpfx)ISO-2022-JP.so: $(objpfx)libKSC.so

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
	      cns11643l1.c cns11643l1.h cns11643.h cns11643.c		    \
	      run-iconv-test.sh $(wildcard testdata/*) ebcdic-dk-no.c	    \
	      ebcdic-dk-no-a.c ebcdic-es.c ebcdic-es-a.c ebcdic-es-s.c	    \
	      ebcdic-fi-se.c ebcdic-fi-se-a.c ebcdic-fr.c ebcdic-is-friss.c \
	      ebcdic-it.c ebcdic-pt.c ebcdic-uk.c ebcdic-us.c ibm037.c	    \
	      ibm038.c ibm256.c ibm273.c ibm274.c ibm275.c ibm277.c	    \
	      ibm278.c ibm280.c ibm281.c ibm284.c ibm285.c ibm290.c	    \
	      ibm297.c ibm420.c ibm423.c ibm424.c ibm437.c ibm500.c	    \
	      ibm850.c ibm851.c ibm852.c ibm855.c ibm857.c ibm860.c	    \
	      ibm861.c ibm862.c ibm863.c ibm864.c ibm865.c ibm868.c	    \
	      ibm869.c ibm870.c ibm871.c ibm875.c ibm880.c ibm891.c	    \
	      ibm903.c ibm904.c ibm905.c ibm918.c ibm1004.c ibm1026.c	    \
	      ibm1047.c cp1250.c cp1251.c cp1252.c cp1253.c cp1254.c	    \
	      cp1255.c cp1256.c cp1257.c cp874.c cp874.h cp737.c cp737.h    \
	      cp775.c cp775.h iso-2022-jp.c iso-2022-kr.c

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
$(make-target-directory)
( echo "static const uint32_t to_ucs4[256] = {"; \
  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\1] = 0x\2,/p' -e d $^ | sort -u; \
  echo "};"; \
  echo "static const char from_ucs4[] = {"; \
  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\2] = 0x\1,/p' -e d $^ | sort -u; \
  echo "};" ) > $@.new && rm -f $@ && mv $@.new $@
endef

sed-generated-headers := iso8859-2.h iso8859-3.h iso8859-4.h iso8859-6.h \
			 iso8859-9.h koi-8.h hp-roman8.h ebcdic-at-de.h \
			 ebcdic-at-de-a.h ebcdic-ca-fr.h ebcdic-dk-no.h  \
			 ebcdic-dk-no-a.h ebcdic-es.h ebcdic-es-a.h	 \
			 ebcdic-es-s.h ebcdic-fi-se.h ebcdic-fi-se-a.h	 \
			 ebcdic-fr.h ebcdic-is-friss.h ebcdic-it.h	 \
			 ebcdic-pt.h ebcdic-uk.h ebcdic-us.h ibm037.h	 \
			 ibm038.h ibm274.h ibm275.h ibm423.h ibm424.h	 \
			 ibm500.h ibm870.h ibm871.h ibm891.h ibm903.h	 \
			 ibm904.h ibm905.h ibm1047.h iso8859-7jp.h

define generate-8bit-gap-table
$(make-target-directory)
( echo "static const uint32_t to_ucs4[256] = {"; \
  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\1] = 0x\2,/p' -e d $^ | sort -u; \
  echo "};"; \
  echo "static const struct gap from_idx[] = {"; \
  sed -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/0x\2 0x\1/p' -e d $^ | sort -u | $(PERL) gap.pl; \
  echo "  { start: 0xffff, end: 0xffff, idx:     0 }"; \
  echo "};"; \
  echo "static const char from_ucs4[] = {"; \
  sed -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/0x\2 0x\1/p' -e d $^ | sort -u | $(PERL) gaptab.pl; \
  echo "};" ) > $@.new && rm -f $@ && mv $@.new $@
endef

perl-generated-headers := koi8-r.h latin-greek.h latin-greek-1.h	 \
			  ibm256.h ibm273.h ibm277.h ibm278.h ibm280.h	 \
			  ibm281.h ibm284.h ibm285.h ibm290.h ibm297.h	 \
			  ibm420.h ibm437.h ibm850.h ibm851.h ibm852.h	 \
			  ibm855.h ibm857.h ibm860.h ibm861.h ibm862.h	 \
			  ibm863.h ibm864.h ibm865.h ibm868.h ibm869.h	 \
			  ibm875.h ibm880.h ibm918.h ibm1004.h ibm1026.h \
			  cp1250.h cp1251.h cp1252.h cp1253.h cp1254.h	 \
			  cp1255.h cp1256.h cp1257.h iso8859-5.h	 \
			  iso8859-7.h iso8859-8.h iso8859-10.h		 \
			  iso8859-7jp.h

# The headers must be generated before the compilation.
before-compile = $(addprefix $(objpfx),$(sed-generated-headers)) \
		 $(addprefix $(objpfx),$(perl-generated-headers))


# Rules to generate the headers.
$(objpfx)iso8859-2.h: ../localedata/charmaps/ISO-8859-2 Makefile
	$(generate-8bit-table)
$(objpfx)iso8859-3.h: ../localedata/charmaps/ISO-8859-3 Makefile
	$(generate-8bit-table)
$(objpfx)iso8859-4.h: ../localedata/charmaps/ISO-8859-4 Makefile
	$(generate-8bit-table)
$(objpfx)iso8859-6.h: ../localedata/charmaps/ISO-8859-6 Makefile
	$(generate-8bit-table)
$(objpfx)iso8859-9.h: ../localedata/charmaps/ISO-8859-9 Makefile
	$(generate-8bit-table)

$(objpfx)koi-8.h: ../localedata/charmaps/KOI-8 Makefile
	$(generate-8bit-table)

$(objpfx)hp-roman8.h: ../localedata/charmaps/HP-ROMAN8 Makefile
	$(generate-8bit-table)

$(objpfx)ebcdic-at-de.h: ../localedata/charmaps/EBCDIC-AT-DE Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-at-de-a.h: ../localedata/charmaps/EBCDIC-AT-DE-A Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-ca-fr.h: ../localedata/charmaps/EBCDIC-CA-FR Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-dk-no.h: ../localedata/charmaps/EBCDIC-DK-NO Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-dk-no-a.h: ../localedata/charmaps/EBCDIC-DK-NO-A Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-es.h: ../localedata/charmaps/EBCDIC-ES Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-es-a.h: ../localedata/charmaps/EBCDIC-ES-A Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-es-s.h: ../localedata/charmaps/EBCDIC-ES-S Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-fi-se.h: ../localedata/charmaps/EBCDIC-FI-SE Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-fi-se-a.h: ../localedata/charmaps/EBCDIC-FI-SE-A Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-fr.h: ../localedata/charmaps/EBCDIC-FR Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-is-friss.h: ../localedata/charmaps/EBCDIC-IS-FRISS Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-it.h: ../localedata/charmaps/EBCDIC-IT Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-pt.h: ../localedata/charmaps/EBCDIC-PT Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-uk.h: ../localedata/charmaps/EBCDIC-UK Makefile
	$(generate-8bit-table)
$(objpfx)ebcdic-us.h: ../localedata/charmaps/EBCDIC-US Makefile
	$(generate-8bit-table)
$(objpfx)ibm037.h: ../localedata/charmaps/IBM037 Makefile
	$(generate-8bit-table)
$(objpfx)ibm038.h: ../localedata/charmaps/IBM038 Makefile
	$(generate-8bit-table)
$(objpfx)ibm274.h: ../localedata/charmaps/IBM274 Makefile
	$(generate-8bit-table)
$(objpfx)ibm275.h: ../localedata/charmaps/IBM275 Makefile
	$(generate-8bit-table)
$(objpfx)ibm423.h: ../localedata/charmaps/IBM423 Makefile
	$(generate-8bit-table)
$(objpfx)ibm500.h: ../localedata/charmaps/IBM500 Makefile
	$(generate-8bit-table)
$(objpfx)ibm870.h: ../localedata/charmaps/IBM870 Makefile
	$(generate-8bit-table)
$(objpfx)ibm871.h: ../localedata/charmaps/IBM871 Makefile
	$(generate-8bit-table)
$(objpfx)ibm891.h: ../localedata/charmaps/IBM891 Makefile
	$(generate-8bit-table)
$(objpfx)ibm903.h: ../localedata/charmaps/IBM903 Makefile
	$(generate-8bit-table)
$(objpfx)ibm904.h: ../localedata/charmaps/IBM904 Makefile
	$(generate-8bit-table)
$(objpfx)ibm905.h: ../localedata/charmaps/IBM905 Makefile
	$(generate-8bit-table)
$(objpfx)ibm1047.h: ../localedata/charmaps/IBM1047 Makefile
	$(generate-8bit-table)

$(objpfx)iso8859-7jp.h: ../localedata/charmaps/ISO-8859-7 Makefile
	$(make-target-directory)
	( echo "static const uint32_t iso88597_to_ucs4[96] = {"; \
	  sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' -e 's/^[^[:space:]]*[[:space:]]*.x\([A-F].\)[[:space:]]*<U\(....\)>.*/  [0x\1 - 0xA0] = 0x\2,/p' -e d $^ | sort -u; \
	  echo "};"; \
	  echo "static const struct gap from_idx[] = {"; \
	  sed -e 's/^[^[:space:]]*[[:space:]]*.x\([A-F].\)[[:space:]]*<U\(....\)>.*/0x\2 0x\1/p' -e d $^ | sort -u | $(PERL) gap.pl; \
	  echo "  { start: 0xffff, end: 0xffff, idx:     0 }"; \
	  echo "};"; \
	  echo "static const char iso88597_from_ucs4[] = {"; \
	  sed -e 's/^[^[:space:]]*[[:space:]]*.x\([A-F].\)[[:space:]]*<U\(....\)>.*/0x\2 0x\1/p' -e d $^ | sort -u | $(PERL) gaptab.pl; \
	  echo "};" ) > $@.new && rm -f $@ && mv $@.new $@


ifneq ($(PERL),no)
$(objpfx)iso8859-5.h: ../localedata/charmaps/ISO-8859-5 Makefile
	$(generate-8bit-gap-table)
$(objpfx)iso8859-7.h: ../localedata/charmaps/ISO-8859-7 Makefile
	$(generate-8bit-gap-table)
$(objpfx)iso8859-8.h: ../localedata/charmaps/ISO-8859-8 Makefile
	$(generate-8bit-gap-table)
$(objpfx)iso8859-10.h: ../localedata/charmaps/ISO-8859-10 Makefile
	$(generate-8bit-gap-table)

$(objpfx)koi8-r.h: ../localedata/charmaps/KOI8-R Makefile
	$(generate-8bit-gap-table)
$(objpfx)latin-greek.h: ../localedata/charmaps/LATIN-GREEK Makefile
	$(generate-8bit-gap-table)
$(objpfx)latin-greek-1.h: ../localedata/charmaps/LATIN-GREEK-1 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm256.h: ../localedata/charmaps/IBM256 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm273.h: ../localedata/charmaps/IBM273 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm277.h: ../localedata/charmaps/IBM277 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm278.h: ../localedata/charmaps/IBM278 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm280.h: ../localedata/charmaps/IBM280 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm281.h: ../localedata/charmaps/IBM281 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm284.h: ../localedata/charmaps/IBM284 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm285.h: ../localedata/charmaps/IBM285 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm290.h: ../localedata/charmaps/IBM290 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm297.h: ../localedata/charmaps/IBM297 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm420.h: ../localedata/charmaps/IBM420 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm424.h: ../localedata/charmaps/IBM424 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm437.h: ../localedata/charmaps/IBM437 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm850.h: ../localedata/charmaps/IBM850 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm851.h: ../localedata/charmaps/IBM851 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm852.h: ../localedata/charmaps/IBM852 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm855.h: ../localedata/charmaps/IBM855 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm857.h: ../localedata/charmaps/IBM857 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm860.h: ../localedata/charmaps/IBM860 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm861.h: ../localedata/charmaps/IBM861 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm862.h: ../localedata/charmaps/IBM862 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm863.h: ../localedata/charmaps/IBM863 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm864.h: ../localedata/charmaps/IBM864 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm865.h: ../localedata/charmaps/IBM865 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm868.h: ../localedata/charmaps/IBM868 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm869.h: ../localedata/charmaps/IBM869 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm875.h: ../localedata/charmaps/IBM875 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm880.h: ../localedata/charmaps/IBM880 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm918.h: ../localedata/charmaps/IBM918 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm1004.h: ../localedata/charmaps/IBM1004 Makefile
	$(generate-8bit-gap-table)
$(objpfx)ibm1026.h: ../localedata/charmaps/IBM1026 Makefile
	$(generate-8bit-gap-table)
$(objpfx)cp1250.h: ../localedata/charmaps/CP1250 Makefile
	$(generate-8bit-gap-table)
$(objpfx)cp1251.h: ../localedata/charmaps/CP1251 Makefile
	$(generate-8bit-gap-table)
$(objpfx)cp1252.h: ../localedata/charmaps/CP1252 Makefile
	$(generate-8bit-gap-table)
$(objpfx)cp1253.h: ../localedata/charmaps/CP1253 Makefile
	$(generate-8bit-gap-table)
$(objpfx)cp1254.h: ../localedata/charmaps/CP1254 Makefile
	$(generate-8bit-gap-table)
$(objpfx)cp1255.h: ../localedata/charmaps/CP1255 Makefile
	$(generate-8bit-gap-table)
$(objpfx)cp1256.h: ../localedata/charmaps/CP1256 Makefile
	$(generate-8bit-gap-table)
$(objpfx)cp1257.h: ../localedata/charmaps/CP1257 Makefile
	$(generate-8bit-gap-table)
endif

headers: $(addprefix $(objpfx),$(sed-generated-headers)) \
	 $(addprefix $(objpfx),$(perl-generated-headers))


$(addprefix $(inst_gconvdir)/, $(modules.so)): \
    $(inst_gconvdir)/%: $(objpfx)% $(+force)
	$(do-install-program)
$(inst_gconvdir)/gconv-modules: gconv-modules $(+force)
	$(do-install)
endif

include ../Rules

.PHONY: do-iconv-test
tests: do-iconv-test

do-iconv-test: run-iconv-test.sh $(objpfx)gconv-modules \
	       $(addprefix $(objpfx),$(modules.so)) \
	       $(common-objdir)/iconv/iconv_prog
	$(SHELL) -e $< $(common-objdir) > $(objpfx)iconv-test.out

$(objpfx)gconv-modules: gconv-modules
	cp $^ $@
