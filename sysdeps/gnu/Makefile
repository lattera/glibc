# Copyright (C) 1996,1997,1998,1999,2001,2002,2003,2004
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

# Generate the list of strings for errno codes from the section of the
# manual which documents the codes.

$(..)sysdeps/gnu/errlist.c: $(..)sysdeps/gnu/errlist.awk \
			    $(..)manual/errno.texi
	$(AWK) -f $^ > $@-tmp
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $@-tmp
	mv -f $@-tmp $@
ifeq ($(with-cvs),yes)
	test ! -d CVS || cvs commit -m'Regenerated from $^' $@
endif

ifeq ($(subdir),stdio-common)

vpath errlist.c $(full_config_sysdirs)
ifeq ($(versioning),yes)
$(objpfx)errlist-compat.c: errlist.c $(..)sysdeps/gnu/errlist-compat.awk \
			   $(common-objpfx)Versions.v.i $(before-compile)
else
$(objpfx)errlist-compat.c: errlist.c $(..)sysdeps/gnu/errlist-compat.awk \
			   $(before-compile)
endif
	$(make-target-directory)
	$(AWK) -v maxerr=`\
	  $(CC) -S $(CPPFLAGS) $(CFLAGS) -DNOT_IN_libc -DEMIT_ERR_MAX $< -o - \
	  | sed -n 's/^.*@@@[^0-9]*\([0-9]*\)[^0-9]*@@@.*$$/\1/p'` \
	       -f $(..)sysdeps/gnu/errlist-compat.awk \
	       $(wildcard $(patsubst %,$(..)%/Versions,\
		 $(config-sysdirs) $(add-ons))) > $@T
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $@T
	mv -f $@T $@
$(objpfx)errlist-compat.h: $(objpfx)errlist-compat.c
	sed -n '1p;/ERR_MAX/p' $< > $@T
	-chmod a-w $@T
	mv -f $@T $@
generated += errlist-compat.c errlist-compat.h

# This will force the generation above to happy if need be.
$(foreach o,.d $(object-suffixes),$(objpfx)errlist$o):$(objpfx)errlist-compat.h
endif

ifeq ($(subdir),login)
sysdep_routines += setutxent getutxent endutxent getutxid getutxline \
                   pututxline utmpxname updwtmpx getutmpx getutmp

sysdep_headers += utmpx.h bits/utmpx.h
endif


ifeq ($(subdir),inet)
sysdep_headers += netinet/udp.h netinet/ip_icmp.h
endif


ifeq ($(subdir),misc)
sysdep_headers += sys/mtio.h
endif
