# Copyright (C) 1991-2007,2009,2010,2011,2012 Free Software Foundation, Inc.
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

#
#	Sub-makefile for POSIX portion of the library.
#
subdir	:= posix

headers	:= sys/utsname.h sys/times.h sys/wait.h sys/types.h unistd.h	      \
	   glob.h regex.h wordexp.h fnmatch.h getopt.h			      \
	   bits/types.h bits/typesizes.h bits/pthreadtypes.h		      \
	   bits/posix1_lim.h bits/posix2_lim.h bits/posix_opt.h		      \
	   bits/local_lim.h tar.h bits/utsname.h bits/confname.h	      \
	   bits/waitflags.h bits/waitstatus.h sys/unistd.h sched.h	      \
	   bits/sched.h re_comp.h wait.h bits/environments.h cpio.h	      \
	   sys/sysmacros.h spawn.h bits/unistd.h

routines :=								      \
	uname								      \
	times								      \
	wait waitpid wait3 wait4 waitid					      \
	alarm sleep pause nanosleep					      \
	fork vfork _exit						      \
	execve fexecve execv execle execl execvp execlp execvpe		      \
	getpid getppid							      \
	getuid geteuid getgid getegid getgroups setuid setgid group_member    \
	getpgid setpgid getpgrp bsd-getpgrp setpgrp getsid setsid	      \
	getresuid getresgid setresuid setresgid				      \
	getlogin getlogin_r setlogin					      \
	pathconf sysconf fpathconf					      \
	glob glob64 fnmatch regex					      \
	confstr								      \
	getopt getopt1 getopt_init					      \
	sched_setp sched_getp sched_sets sched_gets sched_yield sched_primax  \
	sched_primin sched_rr_gi sched_getaffinity sched_setaffinity	      \
	getaddrinfo gai_strerror wordexp				      \
	pread pwrite pread64 pwrite64					      \
	spawn_faction_init spawn_faction_destroy spawn_faction_addclose	      \
	spawn_faction_addopen spawn_faction_adddup2			      \
	spawnattr_init spawnattr_destroy				      \
	spawnattr_getdefault spawnattr_setdefault			      \
	spawnattr_getflags spawnattr_setflags				      \
	spawnattr_getpgroup spawnattr_setpgroup spawn spawnp spawni	      \
	spawnattr_getsigmask spawnattr_getschedpolicy spawnattr_getschedparam \
	spawnattr_setsigmask spawnattr_setschedpolicy spawnattr_setschedparam \
	posix_madvise							      \
	get_child_max sched_cpucount sched_cpualloc sched_cpufree

include ../Makeconfig

aux		:= init-posix environ
tests		:= tstgetopt testfnm runtests runptests	     \
		   tst-preadwrite tst-preadwrite64 test-vfork regexbug1 \
		   tst-getlogin tst-mmap tst-getaddrinfo tst-truncate \
		   tst-truncate64 tst-fork tst-fnmatch tst-regexloc tst-dir \
		   tst-chmod bug-regex1 bug-regex2 bug-regex3 bug-regex4 \
		   tst-gnuglob tst-regex bug-regex5 bug-regex6 bug-regex7 \
		   bug-regex8 bug-regex9 bug-regex10 bug-regex11 bug-regex12 \
		   bug-regex13 bug-regex14 bug-regex15 bug-regex16 \
		   bug-regex17 bug-regex18 bug-regex19 bug-regex20 \
		   bug-regex21 bug-regex22 bug-regex23 bug-regex24 \
		   bug-regex25 bug-regex26 bug-regex27 bug-regex28 \
		   bug-regex29 bug-regex30 bug-regex31 bug-regex32 \
		   bug-regex33 tst-nice tst-nanosleep tst-regex2 \
		   transbug tst-rxspencer tst-pcre tst-boost \
		   bug-ga1 tst-vfork1 tst-vfork2 tst-vfork3 tst-waitid \
		   tst-getaddrinfo2 bug-glob1 bug-glob2 bug-glob3 tst-sysconf \
		   tst-execvp1 tst-execvp2 tst-execlp1 tst-execlp2 \
		   tst-execv1 tst-execv2 tst-execl1 tst-execl2 \
		   tst-execve1 tst-execve2 tst-execle1 tst-execle2 \
		   tst-execvp3 tst-execvp4 tst-rfc3484 tst-rfc3484-2 \
		   tst-rfc3484-3 \
		   tst-getaddrinfo3 tst-fnmatch2 tst-cpucount tst-cpuset \
		   bug-getopt1 bug-getopt2 bug-getopt3 bug-getopt4 \
		   bug-getopt5 tst-getopt_long1
xtests		:= bug-ga2
ifeq (yes,$(build-shared))
test-srcs	:= globtest
tests           += wordexp-test tst-exec tst-spawn
endif
others		:= getconf
install-bin	:= getconf
install-others-programs	:= $(inst_libexecdir)/getconf

before-compile	:= testcases.h ptestcases.h

# So they get cleaned up.
generated := $(addprefix wordexp-test-result, 1 2 3 4 5 6 7 8 9 10) \
	     annexc annexc.out wordexp-tst.out bug-regex2-mem \
	     bug-regex2.mtrace bug-regex14-mem bug-regex14.mtrace \
	     bug-regex21-mem bug-regex21.mtrace \
	     bug-regex31-mem bug-regex31.mtrace \
	     tst-rxspencer-mem tst-rxspencer.mtrace tst-getconf.out \
	     tst-pcre-mem tst-pcre.mtrace tst-boost-mem tst-boost.mtrace \
	     bug-ga2.mtrace bug-ga2-mem bug-glob2.mtrace bug-glob2-mem \
	     tst-vfork3-mem tst-vfork3.mtrace getconf.speclist \
	     tst-fnmatch-mem tst-fnmatch.mtrace

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

# If we will use the generic uname implementation, we must figure out what
# it will say by examining the system, and write the results in config-name.h.
uname.c: $(objpfx)config-name.h
$(objpfx)config-name.h: $(..)scripts/config-uname.sh $(common-objpfx)config.make
	$< '$(config-os)' '$(config-release)' \
	   '$(config-machine)-$(config-vendor)' > $@.new
	mv -f $@.new $@

CFLAGS-regex.c = -Wno-strict-prototypes
CFLAGS-getaddrinfo.c = -DRESOLVER -fexceptions -DUSE_NSCD
CFLAGS-pread.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pread64.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pwrite.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pwrite64.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sleep.c = -fexceptions
CFLAGS-wait.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-waitid.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-waitpid.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-getopt.c = -fexceptions
CFLAGS-wordexp.c = -fexceptions
CFLAGS-wordexp.os = -fomit-frame-pointer
CFLAGS-sysconf.c = -fexceptions -DGETCONF_DIR='"$(libexecdir)/getconf"'
CFLAGS-pathconf.c = -fexceptions
CFLAGS-fpathconf.c = -fexceptions
CFLAGS-spawn.c = -fexceptions
CFLAGS-spawn.os = -fomit-frame-pointer
CFLAGS-spawnp.c = -fexceptions
CFLAGS-spawnp.os = -fomit-frame-pointer
CFLAGS-spawni.c = -fexceptions
CFLAGS-spawni.os = -fomit-frame-pointer
CFLAGS-pause.c = -fexceptions
CFLAGS-glob.c = $(uses-callbacks) -fexceptions
CFLAGS-glob64.c = $(uses-callbacks) -fexceptions
CFLAGS-getconf.c = -DGETCONF_DIR='"$(libexecdir)/getconf"'
CFLAGS-execve.os = -fomit-frame-pointer
CFLAGS-fexecve.os = -fomit-frame-pointer
CFLAGS-execv.os = -fomit-frame-pointer
CFLAGS-execle.os = -fomit-frame-pointer
CFLAGS-execl.os = -fomit-frame-pointer
CFLAGS-execvp.os = -fomit-frame-pointer
CFLAGS-execlp.os = -fomit-frame-pointer

tstgetopt-ARGS = -a -b -cfoobar --required foobar --optional=bazbug \
		--none random --col --color --colour

tst-exec-ARGS = -- $(built-program-cmd)
tst-spawn-ARGS = -- $(built-program-cmd)
tst-dir-ARGS = `pwd` `cd $(common-objdir)/$(subdir); pwd` `cd $(common-objdir); pwd` $(objpfx)tst-dir
tst-chmod-ARGS = $(objdir)
tst-vfork3-ARGS = --test-dir=$(objpfx)

tst-fnmatch-ENV = LOCPATH=$(common-objpfx)localedata
tst-regexloc-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex1-ENV = LOCPATH=$(common-objpfx)localedata
tst-regex-ENV = LOCPATH=$(common-objpfx)localedata
tst-regex2-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex5-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex6-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex17-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex18-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex19-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex20-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex22-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex23-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex25-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex26-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex30-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex32-ENV = LOCPATH=$(common-objpfx)localedata
bug-regex33-ENV = LOCPATH=$(common-objpfx)localedata
tst-rxspencer-ARGS = --utf8 rxspencer/tests
tst-rxspencer-ENV = LOCPATH=$(common-objpfx)localedata
tst-pcre-ARGS = PCRE.tests
tst-boost-ARGS = BOOST.tests
bug-glob1-ARGS = "$(objpfx)"
tst-execvp3-ARGS = --test-dir=$(objpfx)

testcases.h: TESTS TESTS2C.sed
	sed -f TESTS2C.sed < $< > $@T
	mv -f $@T $@

ptestcases.h: PTESTS PTESTS2C.sed
	sed -f PTESTS2C.sed < $< > $@T
	mv -f $@T $@

# Run a test on the header files we use.
# XXX Please note that for now we ignore the result of this test.
tests: $(objpfx)annexc.out
ifeq (no,$(cross-compiling))
tests: $(objpfx)bug-regex2-mem $(objpfx)bug-regex14-mem \
  $(objpfx)bug-regex21-mem $(objpfx)bug-regex31-mem $(objpfx)tst-rxspencer-mem\
  $(objpfx)tst-pcre-mem $(objpfx)tst-boost-mem $(objpfx)tst-getconf.out \
  $(objpfx)bug-glob2-mem $(objpfx)tst-vfork3-mem $(objpfx)tst-fnmatch-mem
xtests: $(objpfx)bug-ga2-mem
endif

$(objpfx)annexc.out: $(objpfx)annexc
	-$(dir $<)$(notdir $<) '$(CC)' \
	  '$(patsubst %,-I../%,$(sorted-subdirs)) -I../include $(+sysdep-includes) $(sysincludes) -I..' > $@

annexc-CFLAGS = -O
$(objpfx)annexc: annexc.c
	$(native-compile)

tst-fnmatch-ENV += MALLOC_TRACE=$(objpfx)tst-fnmatch.mtrace

$(objpfx)tst-fnmatch-mem: $(objpfx)tst-fnmatch.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-fnmatch.mtrace > $@

bug-regex2-ENV = MALLOC_TRACE=$(objpfx)bug-regex2.mtrace

$(objpfx)bug-regex2-mem: $(objpfx)bug-regex2.out
	$(common-objpfx)malloc/mtrace $(objpfx)bug-regex2.mtrace > $@

bug-regex14-ENV = MALLOC_TRACE=$(objpfx)bug-regex14.mtrace

$(objpfx)bug-regex14-mem: $(objpfx)bug-regex14.out
	$(common-objpfx)malloc/mtrace $(objpfx)bug-regex14.mtrace > $@

bug-regex21-ENV = MALLOC_TRACE=$(objpfx)bug-regex21.mtrace

$(objpfx)bug-regex21-mem: $(objpfx)bug-regex21.out
	$(common-objpfx)malloc/mtrace $(objpfx)bug-regex21.mtrace > $@

bug-regex31-ENV = MALLOC_TRACE=$(objpfx)bug-regex31.mtrace

$(objpfx)bug-regex31-mem: $(objpfx)bug-regex31.out
	$(common-objpfx)malloc/mtrace $(objpfx)bug-regex31.mtrace > $@

tst-vfork3-ENV = MALLOC_TRACE=$(objpfx)tst-vfork3.mtrace

$(objpfx)tst-vfork3-mem: $(objpfx)tst-vfork3.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-vfork3.mtrace > $@

# tst-rxspencer.mtrace is generated only when run without --utf8
# option, since otherwise the file has almost 100M and takes very long
# time to process.
$(objpfx)tst-rxspencer-mem: $(objpfx)tst-rxspencer.out
	MALLOC_TRACE=$(objpfx)tst-rxspencer.mtrace $(tst-rxspencer-ENV) \
	  $(run-program-prefix) $(objpfx)tst-rxspencer rxspencer/tests \
	  > /dev/null
	$(common-objpfx)malloc/mtrace $(objpfx)tst-rxspencer.mtrace > $@

tst-pcre-ENV = MALLOC_TRACE=$(objpfx)tst-pcre.mtrace
$(objpfx)tst-pcre-mem: $(objpfx)tst-pcre.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-pcre.mtrace > $@

tst-boost-ENV = MALLOC_TRACE=$(objpfx)tst-boost.mtrace
$(objpfx)tst-boost-mem: $(objpfx)tst-boost.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-boost.mtrace > $@

$(objpfx)tst-getconf.out: tst-getconf.sh $(objpfx)getconf
	$(SHELL) -e $< $(common-objpfx) $(elf-objpfx) $(rtld-installed-name)

ifeq (yes,$(build-shared))
$(objpfx)tst-regex: $(common-objpfx)rt/librt.so
$(objpfx)tst-regex2: $(common-objpfx)rt/librt.so
else
$(objpfx)tst-regex: $(common-objpfx)rt/librt.a
$(objpfx)tst-regex2: $(common-objpfx)rt/librt.a
endif

$(objpfx)bug-ga2-mem: $(objpfx)bug-ga2.out
	$(common-objpfx)malloc/mtrace $(objpfx)bug-ga2.mtrace > $@

bug-ga2-ENV = MALLOC_TRACE=$(objpfx)bug-ga2.mtrace

bug-glob2-ENV = MALLOC_TRACE=$(objpfx)bug-glob2.mtrace

$(objpfx)bug-glob2-mem: $(objpfx)bug-glob2.out
	$(common-objpfx)malloc/mtrace $(objpfx)bug-glob2.mtrace > $@

$(inst_libexecdir)/getconf: $(inst_bindir)/getconf \
			    $(objpfx)getconf.speclist FORCE
	$(addprefix $(..)./scripts/mkinstalldirs ,\
		    $(filter-out $(wildcard $@),$@))
	while read spec; do \
	  ln -f $< $@/$$spec.new || $(INSTALL_PROGRAM) $< $@/$$spec.new; \
	  mv -f $@/$$spec.new $@/$$spec; \
	done < $(objpfx)getconf.speclist

$(objpfx)getconf.speclist: $(objpfx)getconf
ifeq (no,$(cross-compiling))
	LC_ALL=C GETCONF_DIR=/dev/null \
	$(run-program-prefix) $< _POSIX_V7_WIDTH_RESTRICTED_ENVS > $@.new
	LC_ALL=C GETCONF_DIR=/dev/null \
	$(run-program-prefix) $< _POSIX_V6_WIDTH_RESTRICTED_ENVS >> $@.new
	LC_ALL=C GETCONF_DIR=/dev/null \
	$(run-program-prefix) $< _XBS5_WIDTH_RESTRICTED_ENVS >> $@.new
else
	> $@.new
endif
	mv -f $@.new $@
