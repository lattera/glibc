# Copyright (C) 1992, 93, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
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

# Makefile for the GNU C Library manual.

subdir := manual
export subdir := $(subdir)

# Allow override
INSTALL_INFO = install-info

.PHONY: all dvi info
all: dvi
dvi: libc.dvi

# Get glibc's configuration info.
ifneq (,$(wildcard ../Makeconfig))
include ../Makeconfig
else
MAKEINFO = makeinfo
TEXI2DVI = texi2dvi
endif

ifneq ($(strip $(MAKEINFO)),)
all: info
info: libc.info dir-add.info
endif

# Set chapters and chapters-incl[12].
-include chapters
chapters: libc.texinfo
	$(find-includes)
ifdef chapters
# @includes in chapter files
-include chapters-incl1
chapters-incl1: $(chapters)
	$(find-includes)
chapters-incl1 := $(filter-out summary.texi,$(chapters-incl1))
endif
ifdef chapters-incl1
# @includes in files included by chapter files, if any
-include chapters-incl2
chapters-incl2: $(chapters-incl1)
	$(find-includes)
endif

chapters-incl := $(chapters-incl1) $(chapters-incl2)

define find-includes
(echo '$(@F) :=' \\	;\
 $(AWK) '$$1 == "@include" { print $$2 " \\" }' $^) > $@.new
mv -f $@.new $@
endef

# scripts we use
ifndef move-if-change
move-if-change = ./move-if-change
endif
mkinstalldirs = $(..)scripts/mkinstalldirs

libc.dvi libc.info: $(chapters) summary.texi $(chapters-incl)
libc.dvi: texinfo.tex

%.info: %.texinfo
	$(MAKEINFO) $<

%.dvi: %.texinfo
	$(TEXI2DVI) $<

# Generate the summary from the Texinfo source files for each chapter.
summary.texi: stamp-summary ;
stamp-summary: summary.awk $(chapters) $(chapters-incl)
	$(AWK) -f $^ \
	| sort -df +1 -2 | tr '\014' '\012' > summary-tmp
	$(move-if-change) summary-tmp summary.texi
	touch $@

# Generate a file which can be added to the `dir' content to provide direct
# access to the documentation of the function, variables, and other
# definitions.
dir-add.texinfo: xtract-typefun.awk $(chapters) $(chapters-incl)
	(echo "@dircategory GNU C library functions";			\
	 echo "@direntry";						\
	 $(AWK) -f $^;							\
	 echo "@end direntry") > $@.new
	mv -f $@.new $@

# Generate Texinfo files from the C source for the example programs.
%.c.texi: examples/%.c
	sed -e 's,[{}],@&,g'				\
	    -e 's,/\*\(@.*\)\*/,\1,g'			\
	    -e 's,/\*  *,/* @r{,g' -e 's,  *\*/,} */,'	\
	    -e 's/\(@[a-z][a-z]*\)@{\([^}]*\)@}/\1{\2}/g'\
	    $< | expand > $@.new
	mv -f $@.new $@


minimal-dist = summary.awk libc.texinfo $(chapters)			\
	       $(patsubst %.c.texi,examples/%.c,			\
			  $(filter-out summary.texi,$(chapters-incl)))
doc-only-dist = Makefile COPYING.LIB
distribute = $(minimal-dist)	       					\
	     $(patsubst examples/%.c,%.c.texi,$(filter examples/%.c,	\
			$(minimal-dist)))				\
	     libc.info* libc.?? libc.??s texinfo.tex summary.texi	\
	     stamp-summary chapters chapters-incl1 chapters-incl2	\
	     xtract-typefun.awk dir-add.texinfo dir-add.info dir	\
	     stdio-fp.c
export distribute := $(distribute)

tar-it = tar chovf $@ $^

manual.tar: $(doc-only-dist) $(minimal-dist) ; $(tar-it)
mandist.tar: $(doc-only-dist) $(distribute) ; $(tar-it)

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
	-rm -f libc.dvi libc.info* dir-add.info stubs
	-rm -f $(foreach o,$(object-suffixes-for-libc),$(objpfx)stamp$o)
clean: mostlyclean
distclean: clean
indices = cp fn pg tp vr ky
realclean: distclean
	-rm -f chapters chapters-incl* summary.texi stamp-summary *.c.texi
	-rm -f $(foreach index,$(indices),libc.$(index) libc.$(index)s)
	-rm -f libc.log libc.aux libc.toc dir-add.texi

.PHONY: install subdir_install installdirs install-data
install-data subdir_install: install
ifneq ($(strip $(MAKEINFO)),)
install: $(inst_infodir)/libc.info dir-add.info
	@if $(SHELL) -c '$(INSTALL_INFO) --version' >/dev/null 2>&1; then \
	  test -f $(inst_infodir)/dir || cp dir $(inst_infodir);\
	 $(INSTALL_INFO) --info-dir=$(inst_infodir) $(inst_infodir)/libc.info;\
	 $(INSTALL_INFO) --info-dir=$(inst_infodir) dir-add.info;\
	else : ; fi
endif
# Catchall implicit rule for other installation targets from the parent.
install-%: ;

$(inst_infodir)/libc.info: libc.info installdirs
	for file in $<*; do \
	  name=`basename $$file`; \
	  $(INSTALL_DATA) $$file \
	    `echo $@ | sed "s,$<\$$,$$name,"`; \
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
glibc-targets	:= lib objects objs others tests lint.out \
		   echo-headers echo-distinfo
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
$(objpfx)stubs ../po/manual.pot $(objpfx)stamp%:
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
