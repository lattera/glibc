# Makefile for csu code for GNU C library.

# Copyright (C) 1995,1996,1997,1998,1999,2000 Free Software Foundation, Inc.
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

# This directory contains the C startup code (that which calls main).  This
# consists of the startfile, built from start.c and installed as crt0.o
# (traditionally) or crt1.o (for ELF).  In ELF we also install crti.o and
# crtn.o, special "initializer" and "finalizer" files used in the link
# to make the .init and .fini sections work right; both these files are
# built (in an arcane manner) from initfini.c.

subdir := csu

routines = init-first libc-start $(libc-init) sysdep version check_fds
csu-dummies = $(filter-out $(start-installed-name),crt1.o Mcrt1.o)
extra-objs = start.o gmon-start.o \
	     $(start-installed-name) g$(start-installed-name) $(csu-dummies)
omit-deps = $(patsubst %.o,%,$(start-installed-name) g$(start-installed-name) \
			      b$(start-installed-name) $(csu-dummies))
install-lib = $(start-installed-name) g$(start-installed-name) $(csu-dummies)
distribute = initfini.c gmon-start.c start.c defs.awk munch.awk \
	     abi-note.S init.c munch-tmpl.c
generated = version-info.h
before-compile = $(objpfx)version-info.h

all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

ifeq (yes,$(build-bounded))
extra-objs += b$(start-installed-name)
install-lib += b$(start-installed-name)
endif

ifeq (yes,$(elf))
before-compile += $(objpfx)abi-tag.h
generated += abi-tag.h
endif

ifeq (yes,$(gnu-ld))
libc-init = set-init
else
libc-init = munch-init
$(objpfx)munch-init.c: munch.awk munch-tmpl.c $(+subdir_inits)
	$(AWK) -f $< subdirs='$(+init_subdirs)' $(word 2,$^) > $@-t
	mv -f $@-t $@
generated += munch-init.c

# All initialization source files.
+subdir_inits	:= $(wildcard $(foreach dir,$(subdirs),$(dir)/init-$(dir).c))
# All subdirectories containing initialization source files.
+init_subdirs	:= $(patsubst %/,%,$(dir $(+subdir_inits)))
endif

ifeq ($(have-initfini),yes)

CPPFLAGS += -DHAVE_INITFINI

# These are the special initializer/finalizer files.  They are always the
# first and last file in the link.  crti.o ... crtn.o define the global
# "functions" _init and _fini to run the .init and .fini sections.
crtstuff = crti crtn

install-lib += $(crtstuff:=.o)
extra-objs += $(crtstuff:=.o)
generated += $(crtstuff:=.S) initfini.s defs.h
omit-deps += $(crtstuff)

# Special rules for the building of crti.o and crtn.o
$(objpfx)crt%.o: $(objpfx)crt%.S $(objpfx)defs.h
	$(compile.S) -g0 $(ASFLAGS-.os) -o $@

CFLAGS-initfini.s = -g0 -fPIC -fno-inline-functions

vpath initfini.c $(full_config_sysdirs)

$(objpfx)initfini.s: initfini.c
	$(compile.c) -S $(CFLAGS-initfini.s) -finhibit-size-directive \
		$(patsubst -f%,-fno-%,$(exceptions)) -o $@

# We only have one kind of startup code files.  Static binaries and
# shared libraries are build using the PIC version.
$(objpfx)crti.S: $(objpfx)initfini.s
	sed -n -e '1,/@HEADER_ENDS/p' \
	       -e '/@_.*_PROLOG_BEGINS/,/@_.*_PROLOG_ENDS/p' \
	       -e '/@TRAILER_BEGINS/,$$p' $< > $@

$(objpfx)crtn.S: $(objpfx)initfini.s
	sed -n -e '1,/@HEADER_ENDS/p' \
	       -e '/@_.*_EPILOG_BEGINS/,/@_.*_EPILOG_ENDS/p' \
	       -e '/@TRAILER_BEGINS/,$$p' $< > $@

$(objpfx)defs.h: $(objpfx)initfini.s
	sed -n -e '/@TESTS_BEGIN/,/@TESTS_END/p' $< | \
		$(AWK) -f defs.awk > $@

endif

ifeq (yes,$(elf))
extra-objs += abi-note.o init.o
asm-CPPFLAGS += -I$(objpfx).
endif

include ../Rules

# Make these in the lib pass so they're available in time to link things with.
subdir_lib: $(extra-objs:%=$(objpfx)%)

define link-relocatable
$(CC) -nostdlib -nostartfiles -r -o $@ $^
endef

ifndef start-installed-name-rule
ifeq (yes,$(elf))
# We link the ELF startfile along with a SHT_NOTE section indicating
# the kernel ABI the binaries linked with this library will require.
$(objpfx)$(start-installed-name): $(objpfx)start.o $(objpfx)abi-note.o \
				  $(objpfx)init.o
	$(link-relocatable)
$(objpfx)b$(start-installed-name): $(objpfx)start.ob $(objpfx)abi-note.ob \
				  $(objpfx)init.ob
	$(link-relocatable)
else
# The startfile is installed under different names, so we just call our
# source file `start.c' and copy to the installed name after compiling.
$(objpfx)$(start-installed-name): $(objpfx)start.o
	rm -f $@
	ln $< $@
$(objpfx)b$(start-installed-name): $(objpfx)start.ob
	rm -f $@
	ln $< $@
endif
endif

# The profiling startfile is made by linking together the normal
# startfile with gmon-start.o, which defines a constructor function
# to turn on profiling code at startup.
$(objpfx)g$(start-installed-name): $(objpfx)$(start-installed-name) \
				   $(objpfx)gmon-start.o
	$(link-relocatable)

# These extra files are sometimes expected by system standard linking
# procedures, but we have nothing for them to do.  So compile empty files.
$(addprefix $(objpfx),$(filter-out $(start-installed-name),$(csu-dummies))):
	@-rm -f $(@:.o=.c)
	echo > $(@:.o=.c)
	$(COMPILE.c) $(@:.o=.c) $(OUTPUT_OPTION)
	rm -f $(@:.o=.c)

# These headers are used by the startup code.
$(objpfx)abi-tag.h: $(..)abi-tags
	$(make-target-directory)
	rm -f $@.new
	sed -e 's/#.*$$//' -e '/^[ 	]*$$/d' $< | \
	while read conf tag; do \
	  test `expr '$(config-machine)-$(config-vendor)-$(config-os)' \
		     : "$$conf"` != 0 || continue; \
	  echo "$$tag" | \
	  sed -e 's/[^0-9xXa-fA-F]/ /g' -e 's/ *$$//' \
	      -e 's/ /,/g' -e 's/^ */#define ABI_TAG /' > $@.new; \
	done
	if test -r $@.new; then mv -f $@.new $@; \
	else echo >&2 'This configuration not matched in $<'; exit 1; fi

all-Banner-files = $(wildcard $(addsuffix /Banner, \
					$(addprefix $(..), $(subdirs))))
$(objpfx)version-info.h: $(common-objpfx)config.make $(all-Banner-files)
	$(make-target-directory)
	(case $(config-os) in \
	   linux*) version=`(echo -e "#include <linux/version.h>\nUTS_RELEASE"\
			     | $(CC) -E -P - | \
			     sed -e 's/"\([^"]*\)".*/\1/p' -e d) 2>/dev/null`;\
		   if [ -z "$$version" ]; then \
		     if [ -r /proc/version ]; then \
		       version=`sed 's/.*Linux version \([^ ]*\) .*/>>\1<</' \
				< /proc/version`; \
		     else \
		       version=`uname -r`; \
		     fi; \
		   fi; \
		   os=`uname -s 2> /dev/null`; \
		   if [ -z "$$os" ]; then \
		     os=Linux; \
		   fi; \
		   echo "\"Compiled on a $$os $$version system" \
		        "on `date +%Y-%m-%d`.\\n\"" ;; \
	   *) ;; \
	 esac; \
	 files="$(all-Banner-files)";				\
	 if test -n "$$files"; then				\
	   echo "\"Available extensions:";			\
	   sed -e '/^#/d' -e 's/^[[:space:]]*/	/' $$files;	\
	   echo "\"";						\
	 fi) > $@T
	mv -f $@T $@
