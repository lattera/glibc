# Copyright (C) 1992, 1993, 1994, 1995, 1996 Free Software Foundation, Inc.
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

ifeq	($(subdir),math)
ifndef	math-twiddled

sysdep_routines += sincos asincos exp__E log__L
elided-routines += acos asin cos sin

math-twiddled := t
override +gccwarn := -w
endif

endif

ifeq (,$(filter-out $(sysdep_dir)/stub/ $(common-objpfx),\
	$(dir $(firstword $(wildcard $(+sysdep_dirs:%=%/bytesex.h))))))

$(common-objpfx)bytesex.h: $(common-objpfx)det_endian
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

$(common-objpfx)det_endian: $(sysdep_dir)/generic/det_endian.c
	$(native-compile)

before-compile := $(before-compile) $(common-objpfx)bytesex.h
common-generated := $(common-generated) bytesex.h det_endian

endif

ifeq ($(subdir),stdio-common)

ifeq	"$(filter $(objpfx)siglist.c,$(before-compile))" ""
before-compile := $(before-compile) $(objpfx)siglist.c
$(objpfx)siglist.c: $(objpfx)make_siglist
	@rm -f $@
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

$(objpfx)make_siglist: $(sysdep_dir)/generic/make_siglist.c
	$(native-compile) \
	-DSIGNUM_H=\"`cd $(dir $(firstword $(wildcard \
			 $(+sysdep_dirs:%=%/signum.h)))); pwd`/signum.h\"

generated := $(generated) make_siglist siglist.c
endif

endif
