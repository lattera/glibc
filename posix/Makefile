# Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Sub-makefile for POSIX portion of the library.
#
subdir	:= posix

headers	:= sys/utsname.h sys/times.h sys/wait.h sys/types.h unistd.h	\
	   glob.h regex.h wordexp.h fnmatch.h gnu/types.h getopt.h	\
	   posix1_lim.h posix2_lim.h posix_opt.h local_lim.h tar.h	\
	   utsnamelen.h confname.h waitflags.h waitstatus.h sys/unistd.h

distribute := confstr.h

routines :=								      \
	uname								      \
	times								      \
	wait waitpid wait3 wait4					      \
	alarm sleep pause						      \
	fork vfork _exit	      					      \
	execve fexecve execv execle execl execvp execlp			      \
	getpid getppid							      \
	getuid geteuid getgid getegid getgroups setuid setgid group_member    \
	getpgid setpgid getpgrp getsid setsid				      \
	getlogin setlogin						      \
	pathconf sysconf fpathconf					      \
	glob fnmatch regex						      \
	confstr								      \
	getopt getopt1
aux		:= init-posix environ
tests		:= tstgetopt testfnm
others		:= getconf
install-bin	:= getconf
install-lib	:= libposix.a
gpl2lgpl := getopt.c getopt1.c getopt.h	regex.c regex.h

include ../Rules

$(objpfx)libposix.a: $(dep-dummy-lib); $(make-dummy-lib)
lib: $(objpfx)libposix.a

# Make the standalone glob/fnmatch package.

glob.tar: glob/ChangeLog glob/COPYING.LIB \
	  glob/Makefile.in glob/configure glob/configure.in glob/configure.bat\
	  glob/fnmatch.h glob/glob.h glob/fnmatch.c glob/glob.c
	tar cho$(verbose)f $@ $^
glob/%.c: %.c
	rm -f $@
	ln -s ../$< $@
glob/%.h: %.h
	rm -f $@
	ln -s ../$< $@

glob/configure: glob/configure.in
	cd glob; autoconf $(ACFLAGS)
	test ! -d CVS || cvs commit -m'Regenerated: autoconf $(ACFLAGS) $<' $@

glob/ChangeLog: ../ChangeLog
	changelog-extract --regexp 'posix/(glob|fnmatch).*' < $< > $@.new
	chmod a-w $@.new
	mv -f $@.new $@
	test ! -d CVS || cvs commit -mRegenerated $@

%.Z: %
	compress -c $< > $@-tmp
	mv $@-tmp $@
%.gz: %
	gzip -9v -c $< > $@-tmp
	mv $@-tmp $@
