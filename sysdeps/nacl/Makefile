# Makefile fragment for NaCl configurations.

# Copyright (C) 2015-2017 Free Software Foundation, Inc.
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

# The libthread_db code does not compile for NaCl because there is no
# sys/procfs.h supplying the register layout types.  But since libthread_db
# will probably never be useful for NaCl, just elide the directory rather
# than implementing stuff to make it compile (and never get used).
subdirs := $(filter-out nptl_db,$(subdirs))
sorted-subdirs := $(filter-out nptl_db,$(sorted-subdirs))

# The (required) --with-headers option to configure sets sysheaders to the
# location of the native_client/.. source directory.  We'll get necessary
# headers directly from there.
naclsrc = $(sysheaders)/native_client/src

# How to find the directory containing this Makefile.
nacl = $(..)sysdeps/nacl

# Generate our bits/errno.h with the numbers from NaCl's sys/errno.h file.
nacl-errno = $(naclsrc)/trusted/service_runtime/include/sys/errno.h

bits-errno = $(common-objpfx)bits/errno.h
$(bits-errno): $(common-objpfx)stamp-errnos ;
$(common-objpfx)stamp-errnos: $(nacl)/errnos.awk $(..)manual/errno.texi \
			      $(nacl-errno)
	$(make-target-directory)
	$(AWK) -f $^ > $(bits-errno)-tmp
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $(bits-errno)-tmp
	$(move-if-change) $(bits-errno)-tmp $(bits-errno)
	touch $@
common-generated += stamp-errnos bits/errno.h
before-compile += $(bits-errno)

# Massage NaCl's irt.h (and irt_dev.h) into something we can use.
# See irt.sed for details.
nacl-irt.h = $(common-objpfx)nacl-irt.h
$(nacl-irt.h): $(nacl)/irt.sed \
	       $(naclsrc)/untrusted/irt/irt.h \
	       $(naclsrc)/untrusted/irt/irt_dev.h
	sed -f $^ > $@.new
	mv -f $@.new $@
common-generated += nacl-irt.h
before-compile += $(nacl-irt.h)

$(common-objpfx)nacl-interfaces.v.i: $(nacl)/nacl-interfaces.mk.in \
				     $(nacl)/nacl-interface-list.h
-include $(common-objpfx)nacl-interfaces.v
common-generated += nacl-interfaces.v
before-compile += $(common-objpfx)nacl-interfaces.v

nacl-all-interfaces = $(nacl-mandatory-interfaces) $(nacl-optional-interfaces)
nacl-interface-routines = $(nacl-all-interfaces:%=nacl-interface-%)

define nacl-interface-table-command
(echo '#define INTERFACE_CATEGORY $1'; \
 echo '#define INTERFACE_MODULE $(firstword $(subst -, ,$*))'; \
 echo '#define INTERFACE_TYPE $(word 2,$(subst -, ,$*))'; \
 echo '#define INTERFACE_STRING $(nacl-$*-string)'; \
 echo '#include "nacl-interface-table.c"' \
) > $@T
mv -f $@T $@
endef

nacl-interface-pattern = $(objpfx)nacl-interface-%.c

$(nacl-mandatory-interfaces:%=$(nacl-interface-pattern)): \
  $(nacl-interface-pattern): $(nacl)/Makefile $(common-objpfx)nacl-interfaces.v
	$(make-target-directory)
	$(call nacl-interface-table-command,mandatory)
$(nacl-optional-interfaces:%=$(nacl-interface-pattern)): \
  $(nacl-interface-pattern): $(nacl)/Makefile $(common-objpfx)nacl-interfaces.v
	$(make-target-directory)
	$(call nacl-interface-table-command,optional)

nacl-routines-of = $(filter nacl-interface-$1-%,$(nacl-interface-routines))


# Run the NaCl code validator on binaries after we link them, so the
# build does not succeed with any binary that won't pass validation.
# Moving the file around makes sure that we don't leave a target
# appearing complete after it fails validation.
define after-link
mv -f $1 $1.prevalidation
$(nacl)/nacl-after-link.sh '${READELF}' $1.prevalidation
mv -f $1.prevalidation $1
endef

# The test wrapper script takes care of running things under NaCl's sel_ldr.
test-wrapper-env-only = $(nacl)/nacl-test-wrapper.sh --arch=$(nacl-sdk-arch)
test-wrapper-env = $(test-wrapper-env-only)
test-wrapper = $(test-wrapper-env) --

ifeq ($(subdir),csu)
sysdep_routines += nacl_interface_query nacl_interface_ext_supply \
		   nacl-interfaces $(call nacl-routines-of,libc)
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += $(call nacl-routines-of,rtld)
sysdep-rtld-routines += nacl-interfaces $(call nacl-routines-of,rtld)
endif

ifeq ($(subdir),io)
sysdep_routines += xstatconv
endif

ifeq ($(subdir),nptl)
# We do not need any wrappers in libpthread.
libpthread-routines := \
  $(filter-out $(pthread-compat-wrappers),$(libpthread-routines))
endif

ifeq ($(subdir),misc)
# We reuse the Linux file since the bits match.  The file lives in the
# top-level source tree so we can use it without reference to any
# sysdeps/.../linux/ directories, but it's still a sysdeps decision to
# install it.
sysdep_headers += bits/mman-linux.h

# This defeats sysdeps/gnu/Makefile's addition of sys/mtio.h, which
# we do not want.  This is a total kludge, but it seems no worse for
# now than making the sysdeps/gnu/Makefile code conditional on a
# variable we set here.  If some sysdeps/.../Makefile that is later
# in the list than sysdeps/gnu needed to add to sysdep_headers, this
# would break it.  But sysdeps/gnu is close to last in the list and
# this coming up seems unlikely.
override sysdep_headers := $(sysdep_headers)
endif
