# Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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

subdir := mach

all:

# Some things below (but before including Rules) use configuration variables.
include ../Makeconfig


headers = mach_init.h mach.h mach_error.h mach-shortcuts.h \
	  $(interface-headers) mach/mach.h mach/mig_support.h mach/error.h \
	  $(lock-headers) machine-sp.h
distribute = thread_state.h
lock = spin-solid spin-lock mutex-solid
lock-headers = lock-intern.h machine-lock.h spin-lock.h
routines = $(mach-syscalls) $(mach-shortcuts) \
	   mach_init mig_strncpy msg \
	   mig-alloc mig-dealloc mig-reply \
	   msg-destroy msgserver \
	   mach_error errstring error_compat errsystems \
	   devstream bootprivport setup-thread $(lock)
# The RPC interfaces go in a separate library.
interface-library := libmachuser.a
user-interfaces := $(addprefix mach/,mach_interface mach_port mach_host	\
				     memory_object_user			\
				     memory_object_default		\
				     exc mach4				\
	            )\
		   $(addprefix device/,device device_request)
server-interfaces := device/device_reply mach/exc mach/notify
tests := hello
# It is important that we do not use the wildcard function to expand
# `err_*.sub'.  Leaving the wildcard allows Make-dist to find all matching
# files in all sysdep directories.
distribute += Machrules syscalls.awk shortcut.awk \
	      errorlib.h err_*.sub


# Clear any environment	value.
generated =

# Get the proper definition of `mach-srcdir'.
include ../sysdeps/mach/Makefile

# Use and install the Mach header files directly out of the Mach kernel source.

# Find the MiG defs files in the kernel source.
vpath %.defs $(mach-srcdir)

# Install all .h and .defs files we find in some of the kernel's source
# directories and their subdirectories (in MK82, max one level deep).
mach-src-headers := $(wildcard $(foreach dir,mach device mach_debug \
					     $(config-machine),\
					 $(addprefix $(mach-srcdir)/$(dir)/,\
						     *.defs *.h \
						     */*.defs */*.h)))
# Exclude mach/machine/*.  A symlink will be installed for mach/machine.
# Exclude $(headers) for Mach files we have our own versions of.
mach-headers = $(filter-out mach/machine/% $(headers),\
			    $(mach-src-headers:$(mach-srcdir)/%=%))
# Rename Mach's sys/version.h to mach/version.h.
mach-headers := $(patsubst sys/version.h,mach/version.h,$(mach-headers))

# Don't distribute the Mach headers; they are in the Mach distribution.
dont_distribute = $(mach-headers)

# DO NOT try to remake these in any way!!!
$(addprefix $(mach-srcdir)/,$(mach-headers)) : ;
install-others += $(addprefix $(includedir)/,$(mach-headers))
$(includedir)/%: $(mach-srcdir)/%; $(do-install)

# Make symlinks for machine and mach/machine in the include directory.
install-others += $(includedir)/mach/machine $(includedir)/machine
$(includedir)/mach/machine $(includedir)/machine: $(common-objpfx)config.make
	-rm -f $@
	cd $(@D); ln -s $(config-machine) $(@F)

# Install Mach's <sys/version.h> as <mach/version.h>.
install-others += $(includedir)/mach/version.h
$(includedir)/mach/version.h: $(mach-srcdir)/sys/version.h; $(do-install)

# Define mach-syscalls and sysno-*.
ifndef inhibit_mach_syscalls
include $(objpfx)mach-syscalls.mk
endif
$(objpfx)mach-syscalls.mk: mach/syscall_sw.h syscalls.awk
# Go kludges!!!
	sed -n -e '/Unix server implement them/,$$d' \
	       -e 's/^kernel_trap(\(.*\),\([-0-9]*\),\([0-9]*\))$$/\1 \2 \3/p'\
	< $< | awk -f $(word 2,$^) > $@-new
	mv $@-new $@
generated += mach-syscalls.mk

ifndef mach-syscalls
# We cannot make the deps properly until we know how to make the system
# call functions, and for that we must know what they all are.
no_deps=t
else
$(mach-syscalls:%=$(objpfx)%.S): $(objpfx)%.S: $(objpfx)mach-syscalls.mk
	(echo '#include <sysdep.h>'; \
	 echo 'kernel_trap(__$*,$(sysno-$*),$(nargs-$*))'; \
	 echo 'weak_alias (__$*, $*)') > $@-new
	 mv $@-new $@
generated += $(mach-syscalls:=.S)
endif	# mach-syscalls

# syscall_device_writev_request has no RPC equivalent.
mach-shortcuts := $(filter-out device_writev_request,\
		  $(patsubst syscall_%,%,$(filter syscall_%,$(mach-syscalls))))

ifndef mach-shortcuts
# Forget about mach_interface.defs for this run.  On the next run,
# $(mach-shortcuts) will be set, and that will change how
# mach_interface.defs is processed: it will get the -D flags below.
user-interfaces := $(filter-out mach/mach_interface \
				mach/mach_port mach/mach_host mach/mach4 \
				device/device_request,\
				$(user-interfaces))
endif

# Make the MiG stubs for $(mach-shortcuts) be CALL_rpc.
migdefines += $(foreach call,$(mach-shortcuts),-D$(call)=$(call)_rpc)
mach/mach_interface.uh mach/mach_port.uh: $(objpfx)mach-syscalls.mk

ifdef mach-shortcuts
$(mach-shortcuts:%=$(objpfx)%.c): $(objpfx)%.c: shortcut.awk \
						$(objpfx)RPC_%_rpc.c
	gawk -v alias=$* -v call=__$* -v rpc=__$*_rpc -v syscall=__syscall_$* \
	     -f $^ > $@-new
	mv $@-new $@
generated += $(mach-shortcuts:%=%.c)
endif	# mach-shortcuts

# Generate mach-shortcuts.h, which will contain the prototypes for the
# shortcutted kernel RPC functions.
$(objpfx)mach-shortcuts.h: $(objpfx)mach/mach_interface.h \
			   $(objpfx)mach/mach_port.h
# The first line gets us one paragraph per line, with @s separating real lines.
# The second line selects paragraphs for the shortcutted functions.
# The third line removes `_rpc' from the names and rerealifies the lines.
	cat $^ | tr \\012 @ | sed s/@@/@%/g | tr % \\012 \
	| grep '^/\* Routine [a-z0-9_]*_rpc \*/' \
	| sed 's/_rpc//g' | tr @ \\012 > $@-new
	mv $@-new $@
generated += mach-shortcuts.h

before-compile += $(objpfx)mach-shortcuts.h

include Machrules
include ../Rules


# There is already a mach.h, so mach.defs generates mach_interface.h.
$(objpfx)mach/mach_interface.defs: $(mach-srcdir)/mach/mach.defs
	ln $< $@ || cp $< $@
# There is already a memory_object.h,
# so memory_object.defs generates memory_object_user.h.
$(objpfx)mach/memory_object_user.defs: $(mach-srcdir)/mach/memory_object.defs
	ln $< $@ || cp $< $@

ifdef objdir
vpath mach/mach_interface.defs $(objdir)
vpath mach/memory_object_user.defs $(objdir)
endif

# Be sure not to make these with implicit rules from foo.defs.
mach.h mach/memory_object.h: ;

generated += errsystems.c
$(objpfx)errsystems.c: errsystems.awk err_*.sub \
		       $(wildcard $(addsuffix /err_*.sub,$(+sysdep_dirs)))
	gawk -v subsys='$(filter-out $<,$^)' -f $^ > $@.n
	mv $@.n $@
