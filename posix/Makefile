# Copyright (C) 1991-1999, 2000, 2001 Free Software Foundation, Inc.
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

#
#	Sub-makefile for POSIX portion of the library.
#
subdir	:= posix

headers	:= sys/utsname.h sys/times.h sys/wait.h sys/types.h unistd.h	      \
	   glob.h regex.h wordexp.h fnmatch.h bits/types.h getopt.h	      \
	   bits/posix1_lim.h bits/posix2_lim.h bits/posix_opt.h		      \
	   bits/local_lim.h tar.h bits/utsname.h bits/confname.h	      \
	   bits/waitflags.h bits/waitstatus.h sys/unistd.h sched.h	      \
	   bits/sched.h re_comp.h wait.h bits/environments.h cpio.h	      \
	   sys/sysmacros.h spawn.h

distribute := confstr.h TESTS TESTS2C.sed testcases.h \
	      PTESTS PTESTS2C.sed ptestcases.h \
	      globtest.c globtest.sh wordexp-tst.sh annexc.c fnmatch_loop.c   \
	      spawn_int.h tst-getconf.sh

routines :=								      \
	uname								      \
	times								      \
	wait waitpid wait3 wait4 waitid					      \
	alarm sleep pause nanosleep					      \
	fork vfork _exit						      \
	execve fexecve execv execle execl execvp execlp			      \
	getpid getppid							      \
	getuid geteuid getgid getegid getgroups setuid setgid group_member    \
	getpgid setpgid getpgrp bsd-getpgrp setpgrp getsid setsid	      \
	getlogin getlogin_r setlogin					      \
	pathconf sysconf fpathconf					      \
	glob glob64 fnmatch regex					      \
	confstr								      \
	getopt getopt1 getopt_init					      \
	sched_setp sched_getp sched_sets sched_gets sched_yield sched_primax  \
	sched_primin sched_rr_gi					      \
	getaddrinfo gai_strerror wordexp				      \
	pread pwrite pread64 pwrite64					      \
	spawn_faction_init spawn_faction_destroy spawn_faction_addclose	      \
	spawn_faction_addopen spawn_faction_adddup2			      \
	spawnattr_init spawnattr_destroy				      \
	spawnattr_getdefault spawnattr_setdefault			      \
	spawnattr_getflags spawnattr_setflags				      \
	spawnattr_getpgroup spawnattr_setpgroup spawn spawnp spawni	      \
	spawnattr_getsigmask spawnattr_getschedpolicy spawnattr_getschedparam \
	spawnattr_setsigmask spawnattr_setschedpolicy spawnattr_setschedparam

include ../Makeconfig

aux		:= init-posix environ
tests		:= tstgetopt testfnm runtests runptests	     \
		   tst-preadwrite tst-preadwrite64 test-vfork regexbug1 \
		   tst-getlogin tst-mmap tst-getaddrinfo tst-truncate \
		   tst-truncate64 tst-fork tst-fnmatch tst-regexloc tst-dir \
		   tst-chmod bug-regex1 bug-regex2 bug-regex3 bug-regex4 \
		   tst-gnuglob tst-regex bug-regex5
ifeq (yes,$(build-shared))
test-srcs	:= globtest
tests           += wordexp-test tst-exec tst-spawn
endif
others		:= getconf
install-bin	:= getconf
gpl2lgpl := getopt.c getopt1.c getopt.h	regex.c regex.h

before-compile	:= testcases.h ptestcases.h

# So they get cleaned up.
generated := $(addprefix wordexp-test-result, 1 2 3 4 5 6 7 8 9 10) \
	     annexc annexc.out wordexp-tst.out bug-regex2-mem \
	     bug-regex2.mtrace tst-getconf.out

include ../Rules

ifeq (yes,$(build-static-nss))
# We need it for "make check" only. We can skip them if they haven't
# been built yet during "make".
otherlibs += $(wildcard $(nssobjdir)/libnss_files.a \
			$(resolvobjdir)/libnss_dns.a \
			$(resolvobjdir)/libresolv.a)
endif

ifeq (no,$(cross-compiling))
# globtest and wordexp-test currently only works with shared libraries
ifeq (yes,$(build-shared))
tests: $(objpfx)globtest.out $(objpfx)wordexp-tst.out
$(objpfx)globtest.out: globtest.sh $(objpfx)globtest
	$(SHELL) -e globtest.sh $(common-objpfx) $(elf-objpfx) \
		 $(rtld-installed-name)
$(objpfx)wordexp-tst.out: wordexp-tst.sh $(objpfx)wordexp-test
	$(SHELL) -e wordexp-tst.sh $(common-objpfx) $(elf-objpfx) \
		 $(rtld-installed-name)
endif
endif

CFLAGS-regex.c = -Wno-strict-prototypes -DMBS_SUPPORT
CFLAGS-getaddrinfo.c = -DRESOLVER
tstgetopt-ARGS = -a -b -cfoobar --required foobar --optional=bazbug \
		--none random --col --color --colour

tst-exec-ARGS = -- $(built-program-cmd)
tst-spawn-ARGS = -- $(built-program-cmd)
tst-dir-ARGS = `pwd` `cd $(common-objdir)/$(subdir); pwd` `cd $(common-objdir); pwd` $(objpfx)tst-dir
tst-chmod-ARGS = `pwd`

tst-fnmatch-ENV = LOCPATH=$(common-objpfx)localedata
tst-regexloc-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex1-ENV = LOCPATH=$(common-objpfx)localedata
tst-regex-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex5-ENV = LOCPATH=$(common-objpfx)localedata

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

# Run a test on the header files we use.
# XXX Please note that for now we ignore the result of this test.
tests: $(objpfx)annexc.out
ifeq (no,$(cross-compiling))
tests:$(objpfx)bug-regex2-mem $(objpfx)tst-getconf.out
endif

$(objpfx)annexc.out: $(objpfx)annexc
	-$(dir $<)$(notdir $<) '$(CC)' \
			       '-I../include -I.. $(+sysdep-includes) $(sysincludes)' > $@

$(objpfx)annexc: annexc.c
	$(native-compile)

bug-regex2-ENV = MALLOC_TRACE=$(objpfx)bug-regex2.mtrace

$(objpfx)bug-regex2-mem: $(objpfx)bug-regex2.out
	$(common-objpfx)malloc/mtrace $(objpfx)bug-regex2.mtrace > $@

$(objpfx)tst-getconf.out: tst-getconf.sh $(objpfx)getconf
	$(SHELL) -e $< $(common-objpfx) $(elf-objpfx) $(rtld-installed-name)

ifeq (yes,$(build-shared))
$(objpfx)tst-regex: $(common-objpfx)rt/librt.so
else
$(objpfx)tst-regex: $(common-objpfx)rt/librt.a
endif
