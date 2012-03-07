# Copyright (C) 1992-2012
#	Free Software Foundation, Inc.
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

# Makefile for the GNU C Library manual.

subdir := manual
export subdir := $(subdir)

# Allow override
INSTALL_INFO = install-info

.PHONY: all dvi pdf info html
all: dvi
dvi: libc.dvi
pdf: libc.pdf

# Get glibc's configuration info.
ifneq (,$(wildcard ../Makeconfig))
include ../Makeconfig
else
MAKEINFO = makeinfo
TEXI2DVI = texi2dvi
AWK = gawk
endif

TEXI2PDF = texi2dvi --pdf

ifneq ($(strip $(MAKEINFO)),:)
all: info
info: libc.info
endif

# scripts we use
ifndef move-if-change
move-if-change = ./move-if-change
endif
mkinstalldirs = $(..)scripts/mkinstalldirs

chapters = $(addsuffix .texi, \
		       intro errno memory ctype string charset locale	\
		       message search pattern io stdio llio filesys	\
		       pipe socket terminal syslog math arith time	\
		       resource setjmp signal startup process job nss	\
		       users sysinfo conf crypt debug)
add-chapters = $(wildcard $(foreach d, $(add-ons), ../$d/$d.texi))
appendices = lang.texi header.texi install.texi maint.texi contrib.texi
licenses = freemanuals.texi lgpl-2.1.texi fdl-1.3.texi

-include texis
texis: texis.awk $(chapters) $(add-chapters) $(appendices) $(licenses)
	$(AWK) -f $^ > $@.T
	mv -f $@.T $@

nonexamples = $(filter-out $(add-chapters) %.c.texi, $(texis))
examples = $(filter-out $(foreach d, $(add-ons), ../$d/%.c.texi), \
			$(filter %.c.texi, $(texis)))

# Kludge: implicit rule so Make knows the one command does it all.
chapters.% top-menu.%: libc-texinfo.sh $(texis) Makefile
	AWK=$(AWK) $(SHELL) $< '$(chapters)' \
			       '$(add-chapters)' \
			       '$(appendices) $(licenses)'

# Generated files directly included from libc.texinfo.
libc-texi-generated = chapters.texi top-menu.texi dir-add.texi \
		      libm-err.texi version.texi

libc.dvi libc.pdf libc.info: $(libc-texi-generated)
libc.dvi libc.pdf: texinfo.tex

html: libc/index.html
libc/index.html: $(libc-texi-generated)
	$(MAKEINFO) --html libc.texinfo

# Generate the summary from the Texinfo source files for each chapter.
summary.texi: stamp-summary ;
stamp-summary: summary.awk $(filter-out summary.texi, $(texis))
	$(AWK) -f $^ | sort -t'' -df -k 1,1 | tr '\014' '\012' > summary-tmp
	$(move-if-change) summary-tmp summary.texi
	touch $@

# Generate a file which can be added to the `dir' content to provide direct
# access to the documentation of the function, variables, and other
# definitions.
dir-add.texi: xtract-typefun.awk $(texis)
	(echo "@dircategory GNU C library functions and macros";	\
	 echo "@direntry";						\
	 $(AWK) -f $^ | sort;						\
	 echo "@end direntry") > $@.new
	mv -f $@.new $@

# The table with the math errors is generated.
libm-err.texi: stamp-libm-err
stamp-libm-err: libm-err-tab.pl $(wildcard $(foreach dir,$(sysdirs),\
						     $(dir)/libm-test-ulps))
	pwd=`pwd`; \
	$(PERL) $< $$pwd/.. > libm-err-tmp
	$(move-if-change) libm-err-tmp libm-err.texi
	touch $@

# Generate a file with the version number.
version.texi: stamp-version ;
stamp-version: $(common-objpfx)config.make
	echo "@set VERSION $(version)" > version-tmp
	$(move-if-change) version-tmp version.texi
	touch $@

# Generate Texinfo files from the C source for the example programs.
%.c.texi: examples/%.c
	sed -e '1,/^\*\/$$/d'				\
	    -e 's,[{}],@&,g'				\
	    -e 's,/\*\(@.*\)\*/,\1,g'			\
	    -e 's,/\*  *,/* @r{,g' -e 's,  *\*/,} */,'	\
	    -e 's/\(@[a-z][a-z]*\)@{\([^}]*\)@}/\1{\2}/g'\
	    $< | expand > $@.new
	mv -f $@.new $@

%.info: %.texinfo
	LANGUAGE=C LC_ALL=C $(MAKEINFO) $<

%.dvi: %.texinfo
	$(TEXI2DVI) $<

%.pdf: %.texinfo
	$(TEXI2PDF) $<

# Distribution.
minimal-dist = summary.awk texis.awk tsort.awk libc-texinfo.sh libc.texinfo \
	       libm-err.texi stamp-libm-err				    \
	       $(filter-out summary.texi, $(nonexamples))		    \
	       $(patsubst %.c.texi,examples/%.c, $(examples))

tar-it = tar chovf $@ $^

edition := $(shell sed -n 's/^@set EDITION \([0-9][0-9.]*\)[^0-9.]*.*$$/\1/p' \
		       libc.texinfo)

glibc-doc-$(edition).tar: $(doc-only-dist) $(distribute)
	@rm -f glibc-doc-$(edition)
	ln -s . glibc-doc-$(edition)
	tar chovf $@ $(addprefix glibc-doc-$(edition)/,$^)
	rm -f glibc-doc-$(edition)

%.Z: %
	compress -c $< > $@.new
	mv -f $@.new $@
%.gz: %
	gzip -9 -c $< > $@.new
	mv -f $@.new $@
%.uu: %
	uuencode $< < $< > $@.new
	mv -f $@.new $@

.PHONY: mostlyclean distclean realclean clean
mostlyclean:
	-rm -f libc.dvi libc.pdf libc.tmp libc.info*
	-rm -f $(objpfx)stubs
	-rm -f $(foreach o,$(object-suffixes-for-libc),$(objpfx)stamp$o)
clean: mostlyclean
distclean: clean
indices = cp fn pg tp vr ky
realclean: distclean
	-rm -f texis summary.texi stamp-summary *.c.texi
	-rm -f $(foreach index,$(indices),libc.$(index) libc.$(index)s)
	-rm -f libc.log libc.aux libc.toc
	-rm -f $(libc-texi-generated)

.PHONY: install subdir_install installdirs install-data
install-data subdir_install: install
ifneq ($(strip $(MAKEINFO)),:)
install: $(inst_infodir)/libc.info
	@if $(SHELL) -c '$(INSTALL_INFO) --version' >/dev/null 2>&1; then \
	 test -f $(inst_infodir)/dir || $(INSTALL_DATA) dir $(inst_infodir);\
	 $(INSTALL_INFO) --info-dir=$(inst_infodir) $(inst_infodir)/libc.info;\
	else : ; fi
endif
# Catchall implicit rule for other installation targets from the parent.
install-%: ;

$(inst_infodir)/libc.info: libc.info installdirs
	for file in $<*; do \
	  $(INSTALL_DATA) $$file $(@D)/$$file; \
	done

installdirs:
	$(mkinstalldirs) $(inst_infodir)

.PHONY: dist
dist: # glibc-doc-$(edition).tar.gz

ifneq (,$(wildcard ../Make-dist))
dist: ../Make-dist
	$(MAKE) -f $< $(Make-dist-args)
endif

ifndef ETAGS
ETAGS = etags -T
endif
TAGS: $(minimal-dist)
	$(ETAGS) -o $@ $^

# The parent makefile sometimes invokes us with targets `subdir_REAL-TARGET'.
subdir_%: % ;
# For targets we don't define, do nothing.
subdir_%: ;

# These are targets that each glibc subdirectory is expected to understand.
# ../Rules defines them for code subdirectories; for us, they are no-ops.
# None of these should be `subdir_TARGET'; those targets are transformed
# by the implicit rule above into `TARGET' deps.
glibc-targets	:= lib objects objs others tests xtests lint.out echo-headers
.PHONY: $(glibc-targets)
$(glibc-targets):

# Create stamp files if they don't exist, so the parent makefile's rules for
# updating the library archives are happy with us, and never think we have
# changed the library.
lib: $(foreach o,$(object-suffixes-for-libc),$(objpfx)stamp$o)
ifdef objpfx
.PHONY: stubs
stubs: $(objpfx)stubs
endif
$(objpfx)stubs ../po/manual.pot:
	$(make-target-directory)
	touch $@
$(objpfx)stamp%:
	$(make-target-directory)
	touch $@

# Make the target directory if it doesn't exist, using the `mkinstalldirs'
# script that does `mkdir -p' even if `mkdir' doesn't support that flag.
define make-target-directory
$(addprefix $(mkinstalldirs) ,\
	    $(filter-out $(wildcard $(@D:%/=%)),$(@D:%/=%)))
endef

# The top-level glibc Makefile expects subdir_install to update the stubs file.
subdir_install: stubs

# Get rid of these variables if they came from the parent.
routines =
aux =
sources =
objects =
headers =
