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
	   EBCDIC-UK EBCDIC-US IBM037 IBM038 IBM274 IBM275 IBM423	 \
	   IBM500 IBM870 IBM871 IBM891 IBM903 IBM904 IBM905 IBM1047	 \
	   CP874 CP737 CP775 ISO-2022-KR
ifneq ($(PERL),no)
modules += KOI8-R LATIN-GREEK LATIN-GREEK-1 IBM256 IBM273 IBM277 IBM278	 \
	   IBM280 IBM281 IBM284 IBM285 IBM290 IBM297 IBM420 IBM424	 \
	   IBM437 IBM850 IBM851 IBM852 IBM855 IBM857 IBM860 IBM861	 \
	   IBM862 IBM863 IBM864 IBM865 IBM868 IBM869 IBM875 IBM880	 \
	   IBM918 IBM1004 IBM1026 CP1250 CP1251 CP1252 CP1253 CP1254	 \
	   CP1255 CP1256 CP1257 ISO-2022-JP
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
EBCDIC-DK-NO-A-routines := ebcdic-dk-no-a
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
LDFLAGS-ISO-2022-KR.so = -Wl,-rpath,$(gconvdir)
$(objpfx)ISO-2022-KR.so: $(objpfx)libKSC.so

LDFLAGS-libJIS.so = -Wl,-soname,$(@F)
LDFLAGS-libKSC.so = -Wl,-soname,$(@F)
LDFLAGS-libGB.so = -Wl,-soname,$(@F)
LDFLAGS-libCNS.so = -Wl,-soname,$(@F)

distribute := 8bit-generic.c 8bit-gap.c gap.pl gaptab.pl gconv-modules	    \
	      iso8859-1.c iso8859-2.c iso8859-3.c iso8859-4.c iso8859-5.c   \
	      iso8859-6.c iso8859-7.c iso8859-8.c iso8859-9.c iso8859-10.c  \
	      t61.c iso6937.c sjis.c jis0201.h jis0208.h jis0212.h	    \
	      koi-8.c koi8-r.c hp-roman8.c latin-greek.c latin-greek-1.c    \
	      ebcdic-at-de.c ebcdic-at-de-a.c ebcdic-ca-fr.c jis0201.c	    \
	      jis0208.c jis0212.c extra-module.mk euckr.c johab.c uhc.c	    \
	      ksc5601.c ksc5601.h iso646.c big5.c eucjp.c gb2312.c gb2312.h \
	      euccn.c euctw.c cns11643l1.c cns11643l1.h cns11643.h	    \
	      cns11643.c run-iconv-test.sh TESTS $(wildcard testdata/*)	    \
	      ebcdic-dk-no.c ebcdic-dk-no-a.c ebcdic-es.c ebcdic-es-a.c	    \
	      ebcdic-es-s.c ebcdic-fi-se.c ebcdic-fi-se-a.c ebcdic-fr.c	    \
	      ebcdic-is-friss.c ebcdic-it.c ebcdic-pt.c ebcdic-uk.c	    \
	      ebcdic-us.c ibm037.c ibm038.c ibm256.c ibm273.c ibm274.c	    \
	      ibm275.c ibm277.c ibm278.c ibm280.c ibm281.c ibm284.c	    \
	      ibm285.c ibm290.c ibm297.c ibm420.c ibm423.c ibm424.c	    \
	      ibm437.c ibm500.c ibm850.c ibm851.c ibm852.c ibm855.c	    \
	      ibm857.c ibm860.c ibm861.c ibm862.c ibm863.c ibm864.c	    \
	      ibm865.c ibm868.c ibm869.c ibm870.c ibm871.c ibm875.c	    \
	      ibm880.c ibm891.c ibm903.c ibm904.c ibm905.c ibm918.c	    \
	      ibm1004.c ibm1026.c ibm1047.c cp1250.c cp1251.c cp1252.c	    \
	      cp1253.c cp1254.c cp1255.c cp1256.c cp1257.c cp874.c cp874.h  \
	      cp737.c cp737.h cp775.c cp775.h iso-2022-jp.c iso-2022-kr.c   \
	      gen-8bit.sh gen-8bit-gap.sh gen-8bit-gap-1.sh

# We build the transformation modules only when we build shared libs.
ifeq (yes,$(build-shared))

# This macro is similar to build-shlib but it does not define a soname
# and it does not depend on the destination name to start with `lib'.
define build-module
$(LINK.o) -shared -o $@ $(sysdep-LDFLAGS) $(config-LDFLAGS)  \
	  -B$(csu-objpfx) $(load-map-file) \
	  $(LDFLAGS.so) $(LDFLAGS-$(@F:%.so=%).so) \
	  -L$(subst :, -L,$(rpath-link)) -Wl,-rpath-link=$(rpath-link) \
	  -Wl,--whole-archive \
	  $(filter-out $(map-file) $(+preinit) $(+postinit),$^) \
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
move-if-change = ./$(..)scripts/move-if-change

define generate-8bit-table
$(make-target-directory)
$(SHELL) ./gen-8bit.sh $< > $(@:stmp=T)
$(move-if-change) $(@:stmp=T) $(@:stmp=h)
touch $@
endef

sed-generated-headers := iso8859-2.h iso8859-3.h iso8859-4.h iso8859-6.h \
			 iso8859-9.h koi-8.h hp-roman8.h ebcdic-at-de.h  \
			 ebcdic-at-de-a.h ebcdic-ca-fr.h ebcdic-dk-no.h	 \
			 ebcdic-dk-no-a.h ebcdic-es.h ebcdic-es-a.h	 \
			 ebcdic-es-s.h ebcdic-fi-se.h ebcdic-fi-se-a.h	 \
			 ebcdic-fr.h ebcdic-is-friss.h ebcdic-it.h	 \
			 ebcdic-pt.h ebcdic-uk.h ebcdic-us.h ibm037.h	 \
			 ibm038.h ibm274.h ibm275.h ibm423.h ibm500.h	 \
			 ibm870.h ibm871.h ibm891.h ibm903.h ibm904.h	 \
			 ibm905.h ibm1047.h

define generate-8bit-gap-table
$(make-target-directory)
PERL="$(PERL)" $(SHELL) ./gen-8bit-gap.sh $< > $(@:stmp=T)
$(move-if-change) $(@:stmp=T) $(@:stmp=h)
touch $@
endef

perl-generated-headers := koi8-r.h latin-greek.h latin-greek-1.h	 \
			  ibm256.h ibm273.h ibm277.h ibm278.h ibm280.h	 \
			  ibm281.h ibm284.h ibm285.h ibm290.h ibm297.h	 \
			  ibm420.h ibm424.h ibm437.h ibm850.h ibm851.h	 \
			  ibm852.h ibm855.h ibm857.h ibm860.h ibm861.h	 \
			  ibm862.h ibm863.h ibm864.h ibm865.h ibm868.h	 \
			  ibm869.h ibm875.h ibm880.h ibm918.h ibm1004.h	 \
			  ibm1026.h cp1250.h cp1251.h cp1252.h cp1253.h	 \
			  cp1254.h cp1255.h cp1256.h cp1257.h		 \
			  iso8859-5.h iso8859-7.h iso8859-8.h		 \
			  iso8859-10.h iso8859-7jp.h

generated = $(sed-generated-headers) $(sed-generated-headers:%.h=%.stmp) \
	    $(perl-generated-headers) $(perl-generated-headers:%.h=%.stmp) \
	    iconv-test.out
ifdef objpfx
generated += $(objpfx)gconv-modules
endif

# The headers must be generated before the compilation.
before-compile = $(addprefix $(objpfx),$(sed-generated-headers))
ifneq ($(PERL),no)
before-compile += $(addprefix $(objpfx),$(perl-generated-headers))
endif

# Rules to generate the headers.
$(sed-generated-headers:%=$(objpfx)%): $(objpfx)%.h: $(objpfx)%.stmp
	@:

$(objpfx)iso8859-2.stmp: ../localedata/charmaps/ISO-8859-2 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)iso8859-3.stmp: ../localedata/charmaps/ISO-8859-3 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)iso8859-4.stmp: ../localedata/charmaps/ISO-8859-4 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)iso8859-6.stmp: ../localedata/charmaps/ISO-8859-6 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)iso8859-9.stmp: ../localedata/charmaps/ISO-8859-9 gen-8bit.sh
	$(generate-8bit-table)

$(objpfx)koi-8.stmp: ../localedata/charmaps/KOI-8 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)hp-roman8.stmp: ../localedata/charmaps/HP-ROMAN8 gen-8bit.sh
	$(generate-8bit-table)

$(objpfx)ebcdic-at-de.stmp: ../localedata/charmaps/EBCDIC-AT-DE gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-at-de-a.stmp: ../localedata/charmaps/EBCDIC-AT-DE-A gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-ca-fr.stmp: ../localedata/charmaps/EBCDIC-CA-FR gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-dk-no.stmp: ../localedata/charmaps/EBCDIC-DK-NO gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-dk-no-a.stmp: ../localedata/charmaps/EBCDIC-DK-NO-A gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-es.stmp: ../localedata/charmaps/EBCDIC-ES gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-es-a.stmp: ../localedata/charmaps/EBCDIC-ES-A gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-es-s.stmp: ../localedata/charmaps/EBCDIC-ES-S gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-fi-se.stmp: ../localedata/charmaps/EBCDIC-FI-SE gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-fi-se-a.stmp: ../localedata/charmaps/EBCDIC-FI-SE-A gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-fr.stmp: ../localedata/charmaps/EBCDIC-FR gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-is-friss.stmp: ../localedata/charmaps/EBCDIC-IS-FRISS gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-it.stmp: ../localedata/charmaps/EBCDIC-IT gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-pt.stmp: ../localedata/charmaps/EBCDIC-PT gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-uk.stmp: ../localedata/charmaps/EBCDIC-UK gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ebcdic-us.stmp: ../localedata/charmaps/EBCDIC-US gen-8bit.sh
	$(generate-8bit-table)

$(objpfx)ibm037.stmp: ../localedata/charmaps/IBM037 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm038.stmp: ../localedata/charmaps/IBM038 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm274.stmp: ../localedata/charmaps/IBM274 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm275.stmp: ../localedata/charmaps/IBM275 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm423.stmp: ../localedata/charmaps/IBM423 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm500.stmp: ../localedata/charmaps/IBM500 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm870.stmp: ../localedata/charmaps/IBM870 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm871.stmp: ../localedata/charmaps/IBM871 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm891.stmp: ../localedata/charmaps/IBM891 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm903.stmp: ../localedata/charmaps/IBM903 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm904.stmp: ../localedata/charmaps/IBM904 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm905.stmp: ../localedata/charmaps/IBM905 gen-8bit.sh
	$(generate-8bit-table)
$(objpfx)ibm1047.stmp: ../localedata/charmaps/IBM1047 gen-8bit.sh
	$(generate-8bit-table)

ifneq ($(PERL),no)
$(perl-generated-headers:%=$(objpfx)%): $(objpfx)%.h: $(objpfx)%.stmp
	@:

$(objpfx)iso8859-5.stmp: ../localedata/charmaps/ISO-8859-5 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)iso8859-7.stmp: ../localedata/charmaps/ISO-8859-7 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)iso8859-7jp.stmp: ../localedata/charmaps/ISO-8859-7 gen-8bit-gap-1.sh
	$(make-target-directory)
	PERL="$(PERL)" $(SHELL) ./gen-8bit-gap-1.sh $< >$(@:stmp=T)
	$(move-if-change) $(@:stmp=T) $(@:stmp=h)
	touch $@

$(objpfx)iso8859-8.stmp: ../localedata/charmaps/ISO-8859-8 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)iso8859-10.stmp: ../localedata/charmaps/ISO-8859-10 gen-8bit-gap.sh
	$(generate-8bit-gap-table)

$(objpfx)koi8-r.stmp: ../localedata/charmaps/KOI8-R gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)latin-greek.stmp: ../localedata/charmaps/LATIN-GREEK gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)latin-greek-1.stmp: ../localedata/charmaps/LATIN-GREEK-1 gen-8bit-gap.sh
	$(generate-8bit-gap-table)

$(objpfx)ibm256.stmp: ../localedata/charmaps/IBM256 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm273.stmp: ../localedata/charmaps/IBM273 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm277.stmp: ../localedata/charmaps/IBM277 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm278.stmp: ../localedata/charmaps/IBM278 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm280.stmp: ../localedata/charmaps/IBM280 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm281.stmp: ../localedata/charmaps/IBM281 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm284.stmp: ../localedata/charmaps/IBM284 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm285.stmp: ../localedata/charmaps/IBM285 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm290.stmp: ../localedata/charmaps/IBM290 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm297.stmp: ../localedata/charmaps/IBM297 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm420.stmp: ../localedata/charmaps/IBM420 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm424.stmp: ../localedata/charmaps/IBM424 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm437.stmp: ../localedata/charmaps/IBM437 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm850.stmp: ../localedata/charmaps/IBM850 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm851.stmp: ../localedata/charmaps/IBM851 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm852.stmp: ../localedata/charmaps/IBM852 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm855.stmp: ../localedata/charmaps/IBM855 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm857.stmp: ../localedata/charmaps/IBM857 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm860.stmp: ../localedata/charmaps/IBM860 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm861.stmp: ../localedata/charmaps/IBM861 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm862.stmp: ../localedata/charmaps/IBM862 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm863.stmp: ../localedata/charmaps/IBM863 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm864.stmp: ../localedata/charmaps/IBM864 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm865.stmp: ../localedata/charmaps/IBM865 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm868.stmp: ../localedata/charmaps/IBM868 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm869.stmp: ../localedata/charmaps/IBM869 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm875.stmp: ../localedata/charmaps/IBM875 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm880.stmp: ../localedata/charmaps/IBM880 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm918.stmp: ../localedata/charmaps/IBM918 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm1004.stmp: ../localedata/charmaps/IBM1004 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)ibm1026.stmp: ../localedata/charmaps/IBM1026 gen-8bit-gap.sh
	$(generate-8bit-gap-table)

$(objpfx)cp1250.stmp: ../localedata/charmaps/CP1250 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)cp1251.stmp: ../localedata/charmaps/CP1251 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)cp1252.stmp: ../localedata/charmaps/CP1252 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)cp1253.stmp: ../localedata/charmaps/CP1253 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)cp1254.stmp: ../localedata/charmaps/CP1254 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)cp1255.stmp: ../localedata/charmaps/CP1255 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)cp1256.stmp: ../localedata/charmaps/CP1256 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
$(objpfx)cp1257.stmp: ../localedata/charmaps/CP1257 gen-8bit-gap.sh
	$(generate-8bit-gap-table)
endif

headers: $(addprefix $(objpfx),$(sed-generated-headers))
ifneq ($(PERL),no)
headers: $(addprefix $(objpfx),$(perl-generated-headers))
endif


$(addprefix $(inst_gconvdir)/, $(modules.so)): \
    $(inst_gconvdir)/%: $(objpfx)% $(+force)
	$(do-install-program)
$(inst_gconvdir)/gconv-modules: gconv-modules $(+force)
	$(do-install)
endif

include ../Rules

tests: $(objpfx)iconv-test.out

$(objpfx)iconv-test.out: run-iconv-test.sh $(objpfx)gconv-modules \
			 $(addprefix $(objpfx),$(modules.so)) \
			 $(common-objdir)/iconv/iconv_prog TESTS
	$(SHELL) -e $< $(common-objdir) > $@

ifdef objpfx
$(objpfx)gconv-modules: gconv-modules
	cp $^ $@
endif
