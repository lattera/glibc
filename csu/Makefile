# Makefile for csu code for GNU C library.

# Copyright (C) 1995 Free Software Foundation, Inc.
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

# This directory contains the C startup code (that which calls main).  This
# consists of the startfile, built from start.c and installed as crt0.o
# (traditionally) or crt1.o (for ELF); and some initialization code which
# is in the C library itself.  In ELF we also install crti.o and crtn.o,
# special "initializer" and "finalizer" files in used in the link to make
# the .init and .fini sections work right; both these files are built (in
# an arcane manner) from initfini.c.

subdir := csu

csu-dummies = $(filter-out $(start-installed-name),crt1.o Mcrt1.o)
extra-objs = start.o $(start-installed-name) $(csu-dummies)
omit-deps = $(patsubst %.o,%,$(start-installed-name) $(csu-dummies))
install-lib = $(start-installed-name) $(csu-dummies)
distribute = initfini.c

all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

ifneq ($(elf),yes)

# When not using ELF, there is just one startfile, called crt0.o.
start-installed-name = crt0.o

else

# In the ELF universe, crt0.o is called crt1.o, and there are 
# some additional bizarre files.
start-installed-name = crt1.o

# These are the special initializer/finalizer files.  They are always the
# first and last file in the link.  crti.o ... crtn.o are used for normal
# linking; they define the global "functions" _init and _fini to run the
# .init and .fini sections.  crti_s.o ... crtn_s.o are for making shared
# library objects; they put the prologue/epilogue code into the .init and
# .fini sections, but define no global symbols.
crtstuff = crti crtn crti_s crtn_s

install-lib += $(crtstuff:=.o)
extra-objs += $(crtstuff:=.o)
generated += $(crtstuff:=.s)
omit-deps += $(crtstuff)

# Compile initfini.c to assembly code, which contains embedded shell
# commands that prodice crti.s-new and crtn.s-new when run.  We need to
# disable emission of .size directives and debugging information, since
# they will get confused by the splitting of the output we do.
$(objpfx)cr%i.s $(objpfx)cr%n.s: initfini.c; $(initfini)
$(objpfx)cr%i_s.s $(objpfx)cr%n_s.s: initfini.c; $(initfini)

define initfini
-rm -f $(objpfx)crtcommon.tmp
(echo 'cat > crtcommon.tmp <<\EOF_common'; \
 $(CC) $< $(CPPFLAGS) $(CFLAGS) \
       $(patsubst %,-DGLOBAL=static,$(filter %_s.s,$@)) \
       -finhibit-size-directive -g0 -S -o -; \
 echo 'EOF_common') | (cd $(@D); $(SHELL))
cat $(objpfx)crtcommon.tmp >> $(objpfx)crti.s-new
cat $(objpfx)crtcommon.tmp >> $(objpfx)crtn.s-new
rm -f $(objpfx)crtcommon.tmp
mv -f $(objpfx)crti.s-new $(subst crtn,crti,$@)
mv -f $(objpfx)crtn.s-new $(subst crti,crtn,$@)
endef

endif

include ../Rules

# The startfile is installed under different names, so we just call our
# source file `start.c' and copy to the installed name after compiling.
$(objpfx)$(start-installed-name): $(objpfx)start.o
	-rm -f $@
	ln $< $@

# These extra files are sometimes expected by system standard linking
# procedures, but we have nothing for them to do.  So compile empty files.
$(addprefix $(objpfx),$(filter-out $(start-installed-name),$(csu-dummies))):
	cp /dev/null $(@:.o=.c)
	$(COMPILE.c) $(@:.o=.c) $(OUTPUT_OPTION)
	rm -f $(@:.o=.c)

