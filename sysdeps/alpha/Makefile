# Copyright (C) 1993 Free Software Foundation, Inc.
# Contributed by Brendan Kehoe (brendan@zen.org).

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

ifeq ($(subdir),setjmp)
sysdep_routines := $(sysdep_routines) setjmp_aux
endif

ifeq ($(subdir),gnulib)
routines = $(divrem) 
endif	# gnulib

# We distribute these files, even though they are generated,
# so as to avoid the need for a functioning m4 to build the library.
divrem := divl divlu divq divqu reml remlu remq remqu

+divrem-NAME-divl := divl
+divrem-NAME-divlu := divlu
+divrem-NAME-divq := divq
+divrem-NAME-divqu := divqu
+divrem-NAME-reml := reml
+divrem-NAME-remlu := remlu
+divrem-NAME-remq := remq
+divrem-NAME-remqu := remqu
+divrem-NAME = $(+divrem-NAME-$(basename $(notdir $@)))

+divrem-OP-divl := divl
+divrem-OP-divlu := divlu
+divrem-OP-divq := divq
+divrem-OP-divqu := divqu
+divrem-OP-reml := reml
+divrem-OP-remlu := remlu
+divrem-OP-remq := remq
+divrem-OP-remqu := remqu
+divrem-BASEOP-divl := div
+divrem-BASEOP-divlu := div
+divrem-BASEOP-divq := div
+divrem-BASEOP-divqu := div
+divrem-BASEOP-reml := rem
+divrem-BASEOP-remlu := rem
+divrem-BASEOP-remq := rem
+divrem-BASEOP-remqu := rem
+divrem-S-divl := true
+divrem-S-divlu := false
+divrem-S-divq := true
+divrem-S-divqu := false
+divrem-S-reml := true
+divrem-S-remlu := false
+divrem-S-remq := true
+divrem-S-remqu := false
+divrem-SIZE-divl := l
+divrem-SIZE-divlu := l
+divrem-SIZE-divq := q
+divrem-SIZE-divqu := q
+divrem-SIZE-reml := l
+divrem-SIZE-remlu := l
+divrem-SIZE-remq := q
+divrem-SIZE-remqu := q
+divrem-MODE-divl := l
+divrem-MODE-divlu := lu
+divrem-MODE-divq := q
+divrem-MODE-divqu := qu
+divrem-MODE-reml := l
+divrem-MODE-remlu := lu
+divrem-MODE-remq := q
+divrem-MODE-remqu := qu

$(divrem:%=$(sysdep_dir)/alpha/%.S): $(sysdep_dir)/alpha/divrem.m4 $(sysdep_dir)/alpha/DEFS.h $(sysdep_dir)/alpha/macros.m4
	(echo "define(OP,\`$(+divrem-NAME)')\
	       define(BASEOP,\`$(+divrem-BASEOP-$(+divrem-NAME))')\
	       define(MODE,\`$(+divrem-MODE-$(+divrem-NAME))')\
	       define(SIZE,\`$(+divrem-SIZE-$(+divrem-NAME))')\
	       define(SIGNED,\`$(+divrem-S-$(+divrem-NAME))')\
	       define(SYSDEP_DIR, \`$(sysdep_dir)/alpha')\
	       /* This file is generated from divrem.m4; DO NOT EDIT! */"; \
	 cat $<) | $(M4) > $@-tmp
	mv $@-tmp $@
