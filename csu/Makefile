# Makefile for csu code for GNU C library.
# Copyright (C) 1995-2006, 2010, 2012 Free Software Foundation, Inc.
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

# This directory contains the C startup code (that which calls main).  This
# consists of the startfile, built from start.c and installed as crt0.o
# (traditionally) or crt1.o (for ELF).	In ELF we also install crti.o and
# crtn.o, special "initializer" and "finalizer" files used in the link
# to make the .init and .fini sections work right.

subdir := csu

routines = init-first libc-start $(libc-init) sysdep version check_fds \
	   libc-tls elf-init dso_handle
aux	 = errno
elide-routines.os = libc-tls
static-only-routines = elf-init
csu-dummies = $(filter-out $(start-installed-name),crt1.o Mcrt1.o)
extra-objs = start.o gmon-start.o \
	     $(start-installed-name) g$(start-installed-name) $(csu-dummies) \
	     S$(start-installed-name)
omit-deps = $(patsubst %.o,%,$(start-installed-name) g$(start-installed-name) \
			     b$(start-installed-name) $(csu-dummies) \
			     S$(start-installed-name))
install-lib = $(start-installed-name) g$(start-installed-name) $(csu-dummies)
generated = version-info.h
before-compile = $(objpfx)version-info.h

tests := tst-empty tst-atomic tst-atomic-long
tests-static := tst-empty

all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

ifeq (yes,$(build-shared))
extra-objs += S$(start-installed-name)
install-lib += S$(start-installed-name)
generated += start.os
endif

ifeq (yes,$(build-bounded))
extra-objs += b$(start-installed-name)
install-lib += b$(start-installed-name)
generated += start.ob
endif

ifneq ($(start-installed-name),$(static-start-installed-name))
extra-objs += $(static-start-installed-name) g$(static-start-installed-name)
omit-deps += $(patsubst %.o,%,$(static-start-installed-name) \
			     g$(static-start-installed-name))
install-lib += $(static-start-installed-name) g$(static-start-installed-name)
endif

before-compile += $(objpfx)abi-tag.h
generated += abi-tag.h

# These are the special initializer/finalizer files.  They are always the
# first and last file in the link.  crti.o ... crtn.o define the global
# "functions" _init and _fini to run the .init and .fini sections.
crtstuff = crti crtn

install-lib += $(crtstuff:=.o)
extra-objs += $(crtstuff:=.o)

extra-objs += abi-note.o init.o
asm-CPPFLAGS += -I$(objpfx).

include ../Rules

# Make these in the lib pass so they're available in time to link things with.
subdir_lib: $(extra-objs:%=$(objpfx)%)

define link-relocatable
$(CC) -nostdlib -nostartfiles -r -o $@ $^
endef

ifndef start-installed-name-rule
# We link the ELF startfile along with a SHT_NOTE section indicating
# the kernel ABI the binaries linked with this library will require.
$(objpfx)$(start-installed-name): $(objpfx)start.o $(objpfx)abi-note.o \
				  $(objpfx)init.o
	$(link-relocatable)
$(objpfx)S$(start-installed-name): $(objpfx)start.os $(objpfx)abi-note.o \
				  $(objpfx)init.o
	$(link-relocatable)
$(objpfx)b$(start-installed-name): $(objpfx)start.ob $(objpfx)abi-note.ob \
				  $(objpfx)init.ob
	$(link-relocatable)
endif

# The profiling startfile is made by linking together the normal
# startfile with gmon-start.o, which defines a constructor function
# to turn on profiling code at startup.
$(addprefix $(objpfx),$(sort g$(start-installed-name) \
			     g$(static-start-installed-name))): \
  $(objpfx)g%: $(objpfx)% $(objpfx)gmon-start.o
	$(link-relocatable)

# These extra files are sometimes expected by system standard linking
# procedures, but we have nothing for them to do.  So compile empty files.
$(addprefix $(objpfx),$(filter-out $(start-installed-name), $(csu-dummies))):\
  $(before-compile)
	$(COMPILE.c) -o $@ -x c /dev/null

# These headers are used by the startup code.
$(objpfx)abi-tag.h: $(..)abi-tags
	$(make-target-directory)
	rm -f $@.new
	sed -e 's/#.*$$//' -e '/^[	]*$$/d' $< | \
	while read conf tagos tagver; do \
	  test `expr '$(config-machine)-$(config-vendor)-$(config-os)' \
		     : "$$conf"` != 0 || continue; \
	  ( echo "$$tagos" | \
	    sed -e 's/[^0-9xXa-fA-F 	]//' \
		-e 's/^/#define __ABI_TAG_OS /'; \
	    echo "#ifndef __ABI_TAG_VERSION"; \
	    echo "$$tagver" | \
	    sed -e 's/[^0-9xXa-fA-F]/ /g' -e 's/ *$$//' \
	      -e 's/ /,/g' -e 's/^/# define __ABI_TAG_VERSION /'; \
	    echo "#endif" ) > $@.new; \
	done
	if test -r $@.new; then mv -f $@.new $@; \
	else echo >&2 'This configuration not matched in $<'; exit 1; fi

all-Banner-files = $(wildcard $(addsuffix /Banner,\
					  $(sort $(subdir-srcdirs) \
						 $(sysdeps-srcdirs))))
$(objpfx)version-info.h: $(common-objpfx)config.make $(all-Banner-files)
	$(make-target-directory)
	(case $(config-os) in \
	   linux*) version=`(printf '%s\n%s\n' \
				    '#include <linux/version.h>' \
				    UTS_RELEASE \
			     | $(CC) $(CPPFLAGS) -O -E -P - -DNOT_IN_libc=1 | \
			     sed -e 's/"\([^"]*\)".*/\1/p' -e d) 2>/dev/null`;\
		   if [ -z "$$version" ]; then \
		     version=`(printf '%s\n%s\n' \
				      '#include <linux/version.h>' \
				       LINUX_VERSION_CODE \
			       | $(CC) $(CPPFLAGS) -O -E -P - -DNOT_IN_libc=1 \
			       | sed -n -e '/^[123456789].*/p' \
			       | awk '{v=$$1; \
				       printf("%d.%d.%d\n", \
					      v/65535, v/256%256, v%256)}') \
				2>/dev/null`; \
		   fi; \
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
		   printf '"Compiled on a %s %s system on %s.\\n"\n' \
			  "$$os" "$$version" "`date +%Y-%m-%d`";; \
	   *) ;; \
	 esac; \
	 files="$(all-Banner-files)";				\
	 if test -n "$$files"; then				\
	   printf '"Available extensions:\\n"\n';		\
	   sed -e '/^#/d' -e 's/^[[:space:]]*/	/'		\
	       -e 's/^\(.*\)$$/\"\1\\n\"/' $$files;		\
	 fi) > $@T
	mv -f $@T $@
