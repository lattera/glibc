# Copyright (C) 1991, 1992, 1997 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

ifeq ($(subdir),math)

sysdep_routines := $(sysdep_routines) switch

# Find all the sources that have 68881 versions.
+68881-sources := \
  $(notdir $(wildcard $(addprefix $(filter %/fpu,$(sysdirs)),$(sources))))

# Sysdep directories other than fpu and fpu/switch (this one).
+non68881-dirs := $(filter-out %/fpu %/fpu/switch,$(+sysdep_dirs))

# Get a non-68881 version of the target.
+non68881-version = $(firstword $(wildcard $(addsuffix /$@,$(+non68881-dirs))))

# Directory containing 68881 sources.
+68881-dir := $(filter %/fpu,$(+sysdep_dirs))

# For all the files that have 68881 versions and don't exist already in
# the source directory (math), automatically make ones that switch between
# 68881 and soft versions.
$(addprefix $(objpfx), \
	    $(filter-out $(wildcard $(+68881-sources)),$(+68881-sources))):
	(echo '#include <68881-sw.h>'		;\
	 echo '#define $* __$*_68881'		;\
	 echo '#include <$(+68881-dir)/$@>'	;\
	 echo '#undef $*'			;\
	 echo '#define $* __$*_soft'		;\
	 echo '#include <$(non68881-version)>'	;\
	 echo '#undef $*'			;\
	 echo 'switching_function($*);') > $@-tmp
	mv $@-tmp $@

endif
