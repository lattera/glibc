# Copyright (C) 1991, 92, 93, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#
#	Sub-makefile for POSIX portion of the library.
#
subdir	:= posix

headers	:= sys/utsname.h sys/times.h sys/wait.h sys/types.h unistd.h	      \
	   glob.h regex.h wordexp.h fnmatch.h bits/types.h getopt.h	      \
	   bits/posix1_lim.h bits/posix2_lim.h bits/posix_opt.h 	      \
	   bits/local_lim.h tar.h bits/utsname.h bits/confname.h	      \
	   bits/waitflags.h bits/waitstatus.h sys/unistd.h sched.h	      \
	   bits/sched.h re_comp.h wait.h bits/environments.h cpio.h

distribute := confstr.h TESTS TESTS2C.sed testcases.h \
	      PTESTS PTESTS2C.sed ptestcases.h \
	      globtest.c globtest.sh wordexp-tst.sh annexc.c

routines :=								      \
	uname								      \
	times								      \
	wait waitpid wait3 wait4 waitid					      \
	alarm sleep pause nanosleep					      \
	fork vfork _exit	      					      \
	execve fexecve execv execle execl execvp execlp			      \
	getpid getppid							      \
	getuid geteuid getgid getegid getgroups setuid setgid group_member    \
	getpgid setpgid getpgrp bsd-getpgrp setpgrp getsid setsid	      \
	getlogin getlogin_r setlogin					      \
	pathconf sysconf fpathconf					      \
	glob fnmatch regex						      \
	confstr								      \
	getopt getopt1 getopt_init					      \
	sched_setp sched_getp sched_sets sched_gets sched_yield sched_primax  \
	sched_primin sched_rr_gi					      \
	getaddrinfo gai_strerror wordexp				      \
	pread pwrite pread64 pwrite64

aux		:= init-posix environ
tests		:= tstgetopt testfnm runtests wordexp-test runptests
test-srcs	:= globtest
others		:= getconf
install-bin	:= getconf
install-lib	:= libposix.a
gpl2lgpl := getopt.c getopt1.c getopt.h	regex.c regex.h

before-compile	:= testcases.h ptestcases.h

# So they get cleaned up.
generated := $(addprefix wordexp-test-result, 1 2 3 4 5 6 7 8 9 10) \
	     annexc annexc.out

include ../Rules

ifeq (no,$(cross-compiling))
.PHONY: do-globtest do-wordexp-test
tests: do-globtest do-wordexp-test
do-globtest: $(objpfx)globtest
	$(SHELL) -e globtest.sh $(common-objpfx) $(elf-objpfx) \
		 $(rtld-installed-name)
do-wordexp-test: $(objpfx)wordexp-test
	$(SHELL) -e wordexp-tst.sh $(common-objpfx) $(elf-objpfx) \
		 $(rtld-installed-name)
endif

CFLAGS-regex.c = -Wno-unused -Wno-strict-prototypes
CFLAGS-getaddrinfo.c = -DRESOLVER

$(objpfx)libposix.a: $(dep-dummy-lib); $(make-dummy-lib)
lib: $(objpfx)libposix.a

testcases.h: TESTS TESTS2C.sed
	sed -f TESTS2C.sed < $< > $@T
	mv -f $@T $@
ifeq ($(with-cvs),yes)
	test ! -d CVS || cvs $(CVSOPTS) commit -mRegenerated $@
endif

ptestcases.h: PTESTS PTESTS2C.sed
	sed -f PTESTS2C.sed < $< > $@T
	mv -f $@T $@
ifeq ($(with-cvs),yes)
	test ! -d CVS || cvs $(CVSOPTS) commit -mRegenerated $@
endif

# Make the standalone glob/fnmatch package.

glob.tar: glob/ChangeLog glob/COPYING.LIB \
	  glob/Makefile.in glob/configure glob/configure.in glob/configure.bat\
	  glob/SCOPTIONS glob/SMakefile glob/Makefile.ami \
	  glob/fnmatch.h glob/glob.h glob/fnmatch.c glob/glob.c
	tar cho$(verbose)f $@ $^
glob/%.c: %.c
	rm -f $@
	ln -s ../$< $@
glob/%.h: %.h
	rm -f $@
	ln -s ../$< $@

glob/configure: glob/configure.in
	cd glob && autoconf $(ACFLAGS)
ifeq ($(with-cvs),yes)
	test ! -d CVS || cvs $(CVSOPTS) commit -m'Regenerated: autoconf $(ACFLAGS) $<' $@
endif

glob/ChangeLog: ../ChangeLog
	changelog-extract --regexp 'posix/(glob|fnmatch).*' < $< > $@.new
	chmod a-w $@.new
	mv -f $@.new $@
ifeq ($(with-cvs),yes)
	test ! -d CVS || cvs $(CVSOPTS) commit -mRegenerated $@
endif

%.Z: %
	compress -c $< > $@-tmp
	mv $@-tmp $@
%.gz: %
	gzip -9v -c $< > $@-tmp
	mv $@-tmp $@

# Run a test on the header files we use.
# XXX Please note that for now we ignore the result of this test.
tests: $(objpfx)annexc
	-$(dir $<)$(notdir $<) '$(CC)' \
			       '-I../include -I.. $(+sysdep-includes)' > $<.out

$(objpfx)annexc: annexc.c
	$(native-compile)
