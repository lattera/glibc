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

ifeq ($(subdir),gnulib)
routines = dotmul umul $(divrem) alloca
endif	# gnulib

# We distribute these files, even though they are generated,
# so as to avoid the need for a functioning m4 to build the library.
divrem := sdiv udiv rem urem

+divrem-NAME-sdiv := div
+divrem-NAME-udiv := udiv
+divrem-NAME-rem := rem
+divrem-NAME-urem := urem
+divrem-NAME = $(+divrem-NAME-$(basename $(notdir $@)))
+divrem-OP-div := div
+divrem-OP-udiv := div
+divrem-OP-rem := rem
+divrem-OP-urem := rem
+divrem-S-div := true
+divrem-S-rem := true
+divrem-S-udiv := false
+divrem-S-urem := false
$(divrem:%=$(sysdep_dir)/sparc/%.S): $(sysdep_dir)/sparc/divrem.m4
	(echo "define(NAME,\`.$(+divrem-NAME)')\
	       define(OP,\`$(+divrem-OP-$(+divrem-NAME))')\
	       define(S,\`$(+divrem-S-$(+divrem-NAME))')\
	       /* This file is generated from divrem.m4; DO NOT EDIT! */"; \
	 cat $<) | $(M4) > $@-tmp
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $@-tmp
	mv -f $@-tmp $@
	test -d CVS && cvs commit -m'Regenerated from $<' $@

sysdep-realclean := $(sysdep-realclean) $(divrem:%=sysdeps/sparc/%.S)

ifeq ($(subdir),crypt)

ifeq ($(crypt),crypt)
crypt := crypt.sparc # Use crypt/crypt.sparc.S.
endif

endif # crypt
