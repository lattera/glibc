# Copyright (C) 1991,1992,1993,1994,1995,1996,1997,1998,1999,2003,2006,2008,2009
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

# The unix-specific configure fragment writes `unix-generated' in config.make.
config-generated := $(config-generated) $(unix-generated)

ifeq (,$(filter-out $(sysdep_dir)/generic/ $(common-objpfx),\
	$(dir $(firstword $(wildcard $(+sysdep_dirs:%=%/bits/local_lim.h))))))

ifneq (,$(wildcard $(sysincludedir)/sys/param.h))
mk-local_lim-CFLAGS += -DHAVE_SYS_PARAM_H
endif
ifneq (,$(wildcard $(sysincludedir)/sys/limits.h))
mk-local_lim-CFLAGS += -DHAVE_SYS_LIMITS_H
endif
ifneq (,$(wildcard $(sysincludedir)/limits.h))
mk-local_lim-CFLAGS += -DHAVE_LIMITS_H
endif

$(common-objpfx)bits/local_lim.h: $(common-objpfx)mk-local_lim
	$(dir $<)$(notdir $<) > $@-t
	mv -f $@-t $@
$(common-objpfx)mk-local_lim: $(sysdep_dir)/unix/mk-local_lim.c
	$(common-objdir-compile)

before-compile += $(common-objpfx)bits/local_lim.h
common-generated += bits/local_lim.h mk-local_lim

endif

ifeq (,$(filter-out $(sysdep_dir)/generic/ $(common-objpfx),\
	$(patsubst %/sys/param.h,%/,\
		   $(firstword $(wildcard $(+sysdep_dirs:%=%/sys/param.h))))))

before-compile := $(before-compile) $(common-objpfx)sys/param.h

$(common-objpfx)sys/param.h: $(sysincludedir)/sys/param.h
	$(make-target-directory)
	(echo '#ifndef _SYS_PARAM_H';	\
	 echo '#define _SYS_PARAM_H 1';	\
	 echo '#include <endian.h>';		\
	 $(AWK) < $<			\
	   '/^#define[ 	]*NULL/ { print "#ifndef NULL";	\
				  print $$0;		\
				  print "#endif";	\
				  was_null = 1 }	\
	    { if (!was_null) print $$0;			\
	      was_null = 0 }';		\
	 echo '#ifndef MAXHOSTNAMELEN'; \
	 echo '#define MAXHOSTNAMELEN 64 /* XXX */'; \
	 echo '#endif /* No MAXHOSTNAMELEN.  */'; \
	 echo '#endif	/* sys/param.h */') > $@-tmp
	mv $@-tmp $@

$(common-objpfx)param.h.c: $(sysincludedir)/sys/param.h
	rm -f $@
	cp $< $@
$(common-objpfx)param.h.dep: $(common-objpfx)param.h.c \
			     $(sysdep_dir)/unix/Makefile
	$(+mkdep) -I$(sysincludedir) $<			\
	| sed > $@-t					\
	      -e 's@$(sysincludedir)/sys/param.h@@'	\
	      -e 's@^.*:@@'				\
	      -e 's@$<@@g'				\
	      -e 's@$(sysincludedir)/*@@g'		\
	      -e 's@\\$$@@'				\
	      -e 's@^@sys/param.h-includes := $$(sys/param.h-includes) @'
	mv $@-t $@

# Get the generated definition of sys/param.h-includes.
ifndef no_deps
-include $(common-objpfx)param.h.dep
endif

# Don't preempt our own headers.
sys/param.h-includes := \
  $(filter-out $(patsubst $(..)%,%,\
			  $(wildcard $(addprefix $(..),\
				     $(sys/param.h-includes)))),\
	       $(sys/param.h-includes))


.PHONY: sys/param.h-includes
sys/param.h-includes: $(addprefix $(common-objpfx),$(sys/param.h-includes))

ifdef sys/param.h-includes
# Copy the system files to $(common-objdir).
$(addprefix $(common-objpfx),$(sys/param.h-includes)): $(common-objpfx)%: \
						       $(sysincludedir)/%
	$(make-target-directory)
# Some of these files sometimes contain conflicting declarations for htons,
# ntohs, etc.  They also often contain definitions of *_ENDIAN and
# BYTE_ORDER, which we define ourselves in <endian.h>.
	sed -e '/[ 	]*[hn]to[nh][sl][	 (]*/d' \
	    -e '/^#define[ 	]*[A-Z]*_ENDIAN/d' \
	    -e '/^#define[ 	]*BYTE_ORDER/d' \
	    $< > $@-t
	mv $@-t $@
endif

common-generated := $(common-generated) \
		    sys/param.h $(sys/param.h-includes) param.h.c param.h.dep


ifeq ($(subdir),misc)

# Install system headers the system sys/param.h uses.

sysdep_headers := $(sysdep_headers) $(sys/param.h-includes)

endif # misc

endif # No sysdep sys/param.h.

ifeq (,$(filter-out $(sysdep_dir)/generic/ $(common-objpfx),\
	$(dir $(firstword $(wildcard $(+sysdep_dirs:%=%/bits/errno.h))))))

# These need to exist before any compiling is done,
# so cpp doesn't instead find the generic versions.
before-compile := $(before-compile) $(common-objpfx)bits/errno.h

$(common-objpfx)bits/errno.h: $(common-objpfx)make-errnos
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

$(common-objpfx)make-errnos: $(common-objpfx)make-errnos.c
	$(common-objdir-compile)

$(common-objpfx)make-errnos.c: $(sysdep_dir)/unix/errnos-tmpl.c \
			$(sysdep_dir)/unix/errnos.awk $(common-objpfx)errnos
	$(AWK) -f $(word 2,$^) errnos="`tr '\012' ' ' < $(word 3,$^)`" $< > $@T
	mv $@T $@

$(common-objpfx)errnos: $(wildcard $(sysincludedir)/errno.h	\
				   $(sysincludedir)/sys/errno.h)
	sed -n 's/^#define[ 	]*\(E[A-Z0-9][A-Z0-9]*\)[ 	].*$$/\1/p' \
	    $^ > $@-tmp
	mv $@-tmp $@

common-generated := $(common-generated) \
		    errnos.h errnos make-errnos make-errnos.c
endif

ifeq (,$(filter-out $(sysdep_dir)/generic/ $(common-objpfx),\
	$(dir $(firstword $(wildcard $(+sysdep_dirs:%=%/bits/ioctls.h))))))

before-compile := $(before-compile) $(common-objpfx)bits/ioctls.h

$(common-objpfx)bits/ioctls.h: $(common-objpfx)make-ioctls
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

ioctl-includes := sys/termios.h net/nit.h
ioctl-includes := $(wildcard $(addprefix $(sysincludedir)/,$(ioctl-includes)))
make-ioctls-CFLAGS := $(subst /,_,$(subst .,_,\
			$(patsubst $(sysincludedir)/%,-DHAVE_%,\
				   $(ioctl-includes))))

$(common-objpfx)make-ioctls: $(common-objpfx)make-ioctls.c
	$(common-objdir-compile)

$(common-objpfx)make-ioctls.c: $(sysdep_dir)/unix/ioctls-tmpl.c \
			       $(sysdep_dir)/unix/ioctls.awk \
			       $(common-objpfx)ioctls
	$(AWK) -f $(word 2,$^) requests="`cat $(word 3,$^)`" $< > $@T
	mv $@T $@


bits_termios.h := $(firstword $(wildcard $(+sysdep_dirs:%=%/bits/termios.h)))
ifeq ($(bits_termios.h),$(sysdep_dir)/generic/bits/termios.h)
bits_termios.h := # Ignore the generic version.
endif

# If there is a system-specific <bits/termios.h> file, we want to omit
# all the symbols it defines from ioctls.  Otherwise, both
# <bits/ioctls.h> and <bits/termios.h> would define them.  The
# system-specific <bits/termios.h> file presumably defines them with
# the same values as we find from the system's headers.  We also want
# to omit from ioctls the symbols defined in our own
# <sys/ttydefaults.h>, to avoid multiple definition conflicts.  We use
# snarf-ioctls on these files to find what symbols we want to omit.
# fgrep -xv gives all lines which do not match in their entirety;
# without -x, CSTOP's presence elided TIOCSTOP.

$(common-objpfx)ioctls: $(sysdep_dir)/unix/snarf-ioctls \
			$(sysincludedir)/sys/ioctl.h $(ioctl-includes)
	$(dir $<)$(notdir $<) $(filter-out $<,$^) \
	| fgrep -xv "`($(dir $<)$(notdir $<) $(bits_termios.h) \
					     $(..)sysdeps/generic/sys/ttydefaults.h; \
		      echo NULL) \
		     | sort | uniq`" \
	| sort | uniq | tr '\012' ' ' > $@-tmp
	mv $@-tmp $@

common-generated := $(common-generated) \
		    bits/ioctls.h ioctls make-ioctls make-ioctls.c
endif

ifeq (,$(filter-out $(sysdep_dir)/generic/ $(common-objpfx),\
	$(dir $(firstword $(wildcard $(+sysdep_dirs:%=%/sys/syscall.h))))))

# The syscall code assumes a file <sys/syscall.h> that defines macros
# `SYS_call' for syscall `call'.  Variations on this I have seen include:
# it's in <sys/syscall.h>;
# it's in <sys.s>;
# it defines `CALL' instead of `SYS_call'.
# Irix has a <syscall.h> which is not what we want, so check for <sys.s> first.

# Find a file that might have these.  NOTE: This list of possibilities is
# repeated in sysdeps/unix/configure and the two should be kept in sync.
syscall.h := $(firstword $(wildcard $(addprefix $(sysincludedir)/, \
						sys.s sys/sys.s \
						sys.S sys/sys.S \
						syscall.h sys/syscall.h \
						)))
ifdef syscall.h

# Transmogrify any of several formats of the file into the one we want.
$(common-objpfx)sys/syscall.h: $(syscall.h)
	$(make-target-directory)
	tr '[A-Z]' '[a-z]' < $< | \
	sed -e 's/[ 	]sys_/ /' \
	    -e 's/^#define[ 	]*\([a-z0-9_]*\)[ 	]*/#define SYS_\1 /' \
	    -e 's/[ 	]sys_/ SYS_/' \
	    -e 's/SYS_syscall_basenum/syscall_basenum/g' \
	    -e 's/SYS_kerncall_basenum/kerncall_basenum/g' \
	    -e 's/SYS_sysvoffset/sysvoffset/g' \
	    -e '/^#/!d' \
	    -e '/^#ident/d' \
	    -e 's-\(/\*[^*]*\)$$-\1\*/-' \
	> $@-tmp
	mv $@-tmp $@

before-compile += $(common-objpfx)sys/syscall.h
common-generated += sys/syscall.h

endif

endif

ifndef inhibit-unix-syscalls

# Sysdep dirs unix/... can contain a file syscalls.list,
# which specifies objects to be compiled as simple Unix system calls.

-include $(common-objpfx)sysd-syscalls

ifeq (misc,$(subdir))
sysdep_routines += $(unix-extra-syscalls)

ifdef unix-stub-syscalls
# The system call entry points in this list are supposed to be additional
# functions not overriding any other sysdeps/.../call.c implementation, but
# their system call numbers are unavailable in the kernel headers we're
# using.  Instead of a system call stub, these get a function that fails
# with ENOSYS.  We just generate a single module defining one function and
# making all these entry point names aliases for it.
sysdep_routines += stub-syscalls
$(objpfx)stub-syscalls.c: $(common-objpfx)sysd-syscalls \
			  $(..)sysdeps/unix/Makefile
	$(make-target-directory)
	(for call in $(unix-stub-syscalls); do \
	   echo "#define $$call RENAMED_$$call"; \
	 done; \
	 echo '#include <errno.h>'; \
	 for call in $(unix-stub-syscalls); do \
	   echo "#undef $$call"; \
	 done; \
	 echo 'long int _no_syscall (void)'; \
	 echo '{ __set_errno (ENOSYS); return -1L; }'; \
	 for call in $(unix-stub-syscalls); do \
	   case $$call in \
	   *@@*) ver=$${call##*@}; call=$${call%%*@}; \
		 echo "strong_alias (_no_syscall, $${call}_$${ver})"; \
		 echo "default_symbol_version \
		         ($${call}_$${ver}, $$call, $$ver);" ;; \
	   *@@*) ver=$${call##*@}; call=$${call%%*@}; \
		 echo "strong_alias (_no_syscall, $${call}_$${ver})"; \
		 echo "symbol_version ($${call}_$${ver}, $$call, $$ver);" ;; \
	   *) echo "weak_alias (_no_syscall, $$call)"; \
	      echo "stub_warning ($$call)"; \
	      echo "weak_alias (_no_syscall, __GI_$$call)" ;; \
	   esac; \
	 echo '#include <stub-tag.h>'; \
	 done) > $@T
	mv -f $@T $@
generated += stub-syscalls.c
endif
endif

# This is the end of the pipeline for compiling the syscall stubs.
# The stdin is assembler with cpp using sysdep.h macros.
compile-syscall = $(COMPILE.S) -o $@ -x assembler-with-cpp - \
			       $(compile-mkdep-flags)

ifndef avoid-generated
$(common-objpfx)sysd-syscalls: $(..)sysdeps/unix/make-syscalls.sh \
			       $(wildcard $(+sysdep_dirs:%=%/syscalls.list))
	for dir in $(+sysdep_dirs); do \
	  test -f $$dir/syscalls.list && \
	  { sysdirs='$(sysdirs)' \
	    asm_CPP='$(COMPILE.S) -E -x assembler-with-cpp' \
	    $(SHELL) $(dir $<)$(notdir $<) $$dir || exit 1; }; \
	  test $$dir = $(..)sysdeps/unix && break; \
	done > $@T
	mv -f $@T $@
endif

# The $(bppfx)syscall.ob objects depend on s-proto-bp.d, which are
# generated to specify dependencies generated BP stubs have on headers.
# These deps use file names relative to a subdir, so don't
# include them in the parent directory.
ifneq (,$(filter $(unix-syscalls),$(routines) $(sysdep_routines) $(aux)))
ifndef no_deps
-include $(common-objpfx)s-proto-bp.d
endif
endif

$(common-objpfx)s-%.d: $(..)sysdeps/unix/s-%.S \
		       $(wildcard $(+sysdep_dirs:%=%/syscalls.list))
	$(+make-deps)

common-generated += s-proto-bp.d
postclean-generated += sysd-syscalls

endif
