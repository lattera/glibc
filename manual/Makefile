# Makefile for the GNU C Library manual.

# Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

subdir := manual
export subdir := $(subdir)

.PHONY: all dvi info
all: dvi info
dvi: libc.dvi
info: libc.info

# Get glibc's configuration info.
ifneq (,$(wildcard ../Makeconfig))
include ../Makeconfig
endif

# Set chapters and chapters-incl.
include chapters
chapters: libc.texinfo
	$(find-includes)
chapters := $(filter-out summary.texi,$(chapters))
ifdef chapters
include chapters-incl
chapters-incl: $(chapters)
	$(find-includes)
endif

define find-includes
(echo '$(@F) :=' \\	;\
 awk '$$1 == "@include" { print $$2 " \\" }' $^) > $@.new
mv -f $@.new $@
endef

libc.dvi libc.info: $(chapters) summary.texi $(chapters-incl)
libc.dvi: texinfo.tex

# Generate the summary from the Texinfo source files for each chapter.
summary.texi: stamp-summary ;
stamp-summary: summary.awk $(chapters) $(chapters-incl)
	awk -f $^ \
	| sort -df +1 -2 | tr '\014' '\012' > summary-tmp
	./move-if-change summary-tmp summary.texi
# touch is broken on our machines.  Sigh.
	date > $@

# Generate Texinfo files from the C source for the example programs.
%.c.texi: examples/%.c
	sed -e 's,[{}],@&,g'				\
	    -e 's,/\*\(@.*\)\*/,\1,g'			\
	    -e 's,/\*  *,/* @r{,g' -e 's,  *\*/,} */,'	\
	    -e 's/\(@[a-z][a-z]*\)@{\([^}]*\)@}/\1{\2}/'\
	    $< | expand > $@.new
	mv -f $@.new $@


minimal-dist = summary.awk move-if-change libc.texinfo $(chapters)	\
	       $(patsubst %.c.texi,examples/%.c,			\
			  $(filter-out summary.texi,$(chapters-incl)))
doc-only-dist = Makefile COPYING.LIB mkinstalldirs
distribute = $(minimal-dist)	       					\
	     $(patsubst examples/%.c,%.c.texi,$(filter examples/%.c,	\
			$(minimal-dist)))				\
	     libc.?? libc.??s texinfo.tex summary.texi			\
	     stamp-summary chapters chapters-incl
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

# The parent makefile sometimes invokes us with targets `subdir_REAL-TARGET'.
subdir_%: % ;

.PHONY: mostlyclean distclean realclean clean
mostlyclean:
	-rm -f libc.dvi libc.info*
clean: mostlyclean
distclean: clean
indices = cp fn pg tp vr ky
realclean: distclean
	-rm -f chapters chapters-incl summary.texi stamp-summary *.c.texi
	-rm -f $(foreach index,$(indices),libc.$(index) libc.$(index)s)
	-rm -f libc.log libc.aux libc.toc

.PHONY: install subdir_install installdirs install-data
install-data subdir_install: install
install: $(infodir)/libc.info
# Catchall implicit rule for other installation targets from the parent.
install-%: ;

ifndef infodir
infodir = $(prefix)/info
endif
ifndef prefix
prefix = /usr/local
endif

ifndef INSTALL_DATA
INSTALL_DATA = $(INSTALL) -m 644
endif
ifndef INSTALL
INSTALL = install
endif

$(infodir)/libc.info: libc.info installdirs
	for file in $<*; do \
	  name=`basename $$file`; \
	  $(INSTALL_DATA) $$file \
	    `echo $@ | sed "s,$<\$$,$$name,"`; \
	done

installdirs: $(firstword $(wildcard mkinstalldirs ../mkinstalldirs))
	$(dir $<)$(notdir $<) $(infodir)

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

# These are targets that each glibc subdirectory is expected to understand.
# ../Rules defines them for code subdirectories; for us, they are no-ops.
glibc-targets	:= subdir_lib objects objs others tests subdir_lint.out \
		   subdir_echo-headers subdir_echo-distinfo stubs
.PHONY: $(glibc-targets)
$(glibc-targets):

stubs: $(common-objpfx)stub-manual
$(common-objpfx)stub-manual:
	cp /dev/null $@

# The top-level glibc Makefile expects subdir_install to update the stubs file.
subdir_install: stubs


# Get rid of these variables if they came from the parent.
routines =
aux =
sources =
objects =
headers =
