# Copyright (C) 1999-2015 Free Software Foundation, Inc.
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
#	Sub-makefile for conform portion of the library.
#
subdir	:= conform

include ../Makeconfig

conformtest-headers-data := $(wildcard data/*.h-data) \
			    $(wildcard data/*/*.h-data)

conformtest-standards := ISO ISO99 ISO11 POSIX XPG3 XPG4 UNIX98 XOPEN2K \
			 POSIX2008 XOPEN2K8

conformtest-headers-ISO := assert.h ctype.h errno.h float.h limits.h locale.h \
			   math.h setjmp.h signal.h stdarg.h stddef.h stdio.h \
			   stdlib.h string.h time.h
conformtest-headers-ISO99 := $(conformtest-headers-ISO) complex.h fenv.h \
			     inttypes.h iso646.h stdbool.h stdint.h tgmath.h \
			     wchar.h wctype.h
# Missing ISO11 expectations for: stdatomic.h threads.h.
conformtest-headers-ISO11 := $(conformtest-headers-ISO99) stdalign.h \
			     stdnoreturn.h uchar.h
conformtest-headers-POSIX := $(conformtest-headers-ISO) aio.h dirent.h \
			     fcntl.h fnmatch.h glob.h grp.h mqueue.h \
			     pthread.h pwd.h regex.h sched.h semaphore.h \
			     sys/mman.h sys/stat.h sys/times.h sys/types.h \
			     sys/utsname.h sys/wait.h tar.h termios.h \
			     unistd.h utime.h wordexp.h
# Missing XPG3 expectations for: regexp.h wchar.h.
conformtest-headers-XPG3 := $(conformtest-headers-ISO) cpio.h dirent.h \
			    fcntl.h fnmatch.h ftw.h glob.h grp.h iconv.h \
			    langinfo.h monetary.h nl_types.h pwd.h regex.h \
			    search.h sys/ipc.h sys/msg.h sys/sem.h sys/shm.h \
			    sys/stat.h sys/times.h sys/types.h sys/utsname.h \
			    sys/wait.h tar.h termios.h ulimit.h unistd.h \
			    utime.h varargs.h wordexp.h
# Missing XPG4 expectations for: re_comp.h regexp.h wchar.h.
# XPG4 includes XTI, but xti.h is outside the scope of these tests.
conformtest-headers-XPG4 := $(conformtest-headers-XPG3) arpa/inet.h fmtmsg.h \
			    libgen.h ndbm.h netdb.h netinet/in.h poll.h \
			    strings.h stropts.h sys/mman.h sys/resource.h \
			    sys/socket.h sys/statvfs.h sys/time.h sys/timeb.h \
			    sys/uio.h sys/un.h syslog.h ucontext.h utmpx.h
# Missing UNIX98 expectations for: inttypes.h re_comp.h regexp.h.
# The online UNIX98 includes XCURSES, but curses.h, term.h and
# unctrl.h are outside the scope of these tests.  It also includes
# XTI, but xti.h is outside the scope of these tests.
conformtest-headers-UNIX98 := $(conformtest-headers-POSIX) arpa/inet.h cpio.h \
			      dlfcn.h fmtmsg.h ftw.h iconv.h iso646.h \
			      langinfo.h libgen.h monetary.h ndbm.h netdb.h \
			      netinet/in.h nl_types.h poll.h search.h \
			      strings.h stropts.h sys/ipc.h sys/msg.h \
			      sys/resource.h sys/sem.h sys/shm.h sys/socket.h \
			      sys/statvfs.h sys/time.h sys/timeb.h sys/uio.h \
			      sys/un.h syslog.h ucontext.h ulimit.h utmpx.h \
			      varargs.h wchar.h wctype.h
# Missing XOPEN2K expectations for: trace.h.
conformtest-headers-XOPEN2K := $(conformtest-headers-POSIX) arpa/inet.h \
			       complex.h cpio.h dlfcn.h fenv.h fmtmsg.h ftw.h \
			       iconv.h inttypes.h iso646.h langinfo.h \
			       libgen.h monetary.h ndbm.h net/if.h netdb.h \
			       netinet/in.h netinet/tcp.h nl_types.h poll.h \
			       search.h spawn.h stdbool.h stdint.h strings.h \
			       stropts.h sys/ipc.h sys/msg.h sys/resource.h \
			       sys/select.h sys/sem.h sys/shm.h sys/socket.h \
			       sys/statvfs.h sys/time.h sys/timeb.h sys/uio.h \
			       sys/un.h syslog.h tgmath.h ucontext.h ulimit.h \
			       utmpx.h wchar.h wctype.h
# Missing POSIX2008 expectations for: trace.h.
conformtest-headers-POSIX2008 := $(conformtest-headers-POSIX) arpa/inet.h \
				 complex.h cpio.h dlfcn.h fenv.h iconv.h \
				 inttypes.h iso646.h langinfo.h monetary.h \
				 net/if.h netdb.h netinet/in.h netinet/tcp.h \
				 nl_types.h poll.h spawn.h stdbool.h stdint.h \
				 strings.h stropts.h sys/select.h \
				 sys/socket.h sys/statvfs.h sys/un.h tgmath.h \
				 wchar.h wctype.h
# Missing XOPEN2K8 expectations for: trace.h.
conformtest-headers-XOPEN2K8 := $(conformtest-headers-POSIX2008) fmtmsg.h \
				ftw.h libgen.h ndbm.h search.h sys/ipc.h \
				sys/msg.h sys/resource.h sys/sem.h sys/shm.h \
				sys/time.h sys/uio.h syslog.h ulimit.h utmpx.h

conformtest-header-list-base := $(foreach std,$(conformtest-standards),\
					      header-list-$(std).out)
conformtest-header-list-tests := $(addprefix $(objpfx),\
					     $(conformtest-header-list-base))
tests-special += $(conformtest-header-list-tests)
generated += $(conformtest-header-list-base)

conformtest-header-base := $(foreach std,\
				     $(conformtest-standards),\
				     $(foreach h,\
					       $(conformtest-headers-$(std)),\
					       $(std)/$(h)/conform.out))
conformtest-header-tests := $(addprefix $(objpfx),$(conformtest-header-base))
ifneq (yes,$(fast-check))
ifeq (no,$(cross-compiling))
tests-special += $(conformtest-header-tests)
generated += $(conformtest-header-base)
endif
endif

linknamespace-symlists-base := $(foreach std,$(conformtest-standards),\
					     symlist-$(std))
linknamespace-symlists-tests := $(addprefix $(objpfx),\
					    $(linknamespace-symlists-base))
tests-special += $(linknamespace-symlists-tests)

linknamespace-symlist-stdlibs-base := $(foreach std,$(conformtest-standards),\
						    symlist-stdlibs-$(std))
linknamespace-symlist-stdlibs-tests := \
	$(addprefix $(objpfx),\
		    $(linknamespace-symlist-stdlibs-base))

tests-special += $(linknamespace-symlist-stdlibs-tests)

linknamespace-header-base := $(foreach std,\
				       $(conformtest-standards),\
				       $(foreach h,\
						 $(conformtest-headers-$(std)),\
						 $(std)/$(h)/linknamespace.out))
linknamespace-header-tests := $(addprefix $(objpfx),\
					  $(linknamespace-header-base))
tests-special += $(linknamespace-header-tests)

include ../Rules

$(conformtest-header-list-tests): $(objpfx)header-list-%.out: \
				  check-header-lists.sh \
				  $(conformtest-headers-data)
	$(SHELL) $< "$*" "$(CC)" "$(strip $(conformtest-headers-$*))" \
		 "$(conformtest-headers-data)" > $@; \
	$(evaluate-test)

# GCC 4.6 and earlier lack features required by these tests.
test-xfail-ISO11/complex.h/conform = yes
test-xfail-ISO11/stdalign.h/conform = yes
test-xfail-ISO11/stdnoreturn.h/conform = yes

# Pre-standard C feature no longer supported by GCC (obsoleted in
# newer POSIX standards).
test-xfail-XPG3/varargs.h/conform = yes
test-xfail-XPG4/varargs.h/conform = yes
test-xfail-UNIX98/varargs.h/conform = yes

# Header not provided by glibc.
test-xfail-XPG4/ndbm.h/conform = yes
test-xfail-UNIX98/ndbm.h/conform = yes
test-xfail-XOPEN2K/ndbm.h/conform = yes
test-xfail-XOPEN2K8/ndbm.h/conform = yes

# Unsorted expected failures.
test-xfail-XPG3/fcntl.h/conform = yes
test-xfail-XPG3/ftw.h/conform = yes
test-xfail-XPG3/grp.h/conform = yes
test-xfail-XPG3/langinfo.h/conform = yes
test-xfail-XPG3/limits.h/conform = yes
test-xfail-XPG3/pwd.h/conform = yes
test-xfail-XPG3/search.h/conform = yes
test-xfail-XPG3/signal.h/conform = yes
test-xfail-XPG3/stdio.h/conform = yes
test-xfail-XPG3/stdlib.h/conform = yes
test-xfail-XPG3/string.h/conform = yes
test-xfail-XPG3/sys/ipc.h/conform = yes
test-xfail-XPG3/sys/msg.h/conform = yes
test-xfail-XPG3/sys/sem.h/conform = yes
test-xfail-XPG3/sys/shm.h/conform = yes
test-xfail-XPG3/sys/stat.h/conform = yes
test-xfail-XPG3/sys/types.h/conform = yes
test-xfail-XPG3/sys/wait.h/conform = yes
test-xfail-XPG3/termios.h/conform = yes
test-xfail-XPG3/time.h/conform = yes
test-xfail-XPG3/unistd.h/conform = yes
test-xfail-XPG4/arpa/inet.h/conform = yes
test-xfail-XPG4/fcntl.h/conform = yes
test-xfail-XPG4/langinfo.h/conform = yes
test-xfail-XPG4/netdb.h/conform = yes
test-xfail-XPG4/netinet/in.h/conform = yes
test-xfail-XPG4/signal.h/conform = yes
test-xfail-XPG4/stdio.h/conform = yes
test-xfail-XPG4/stdlib.h/conform = yes
test-xfail-XPG4/stropts.h/conform = yes
test-xfail-XPG4/sys/ipc.h/conform = yes
test-xfail-XPG4/sys/msg.h/conform = yes
test-xfail-XPG4/sys/sem.h/conform = yes
test-xfail-XPG4/sys/shm.h/conform = yes
test-xfail-XPG4/sys/socket.h/conform = yes
test-xfail-XPG4/sys/stat.h/conform = yes
test-xfail-XPG4/sys/time.h/conform = yes
test-xfail-XPG4/sys/types.h/conform = yes
test-xfail-XPG4/sys/wait.h/conform = yes
test-xfail-XPG4/termios.h/conform = yes
test-xfail-XPG4/ucontext.h/conform = yes
test-xfail-XPG4/unistd.h/conform = yes
test-xfail-XPG4/utmpx.h/conform = yes
test-xfail-POSIX/sys/wait.h/conform = yes
test-xfail-UNIX98/arpa/inet.h/conform = yes
test-xfail-UNIX98/fcntl.h/conform = yes
test-xfail-UNIX98/langinfo.h/conform = yes
test-xfail-UNIX98/netdb.h/conform = yes
test-xfail-UNIX98/netinet/in.h/conform = yes
test-xfail-UNIX98/signal.h/conform = yes
test-xfail-UNIX98/stdio.h/conform = yes
test-xfail-UNIX98/stdlib.h/conform = yes
test-xfail-UNIX98/stropts.h/conform = yes
test-xfail-UNIX98/sys/ipc.h/conform = yes
test-xfail-UNIX98/sys/msg.h/conform = yes
test-xfail-UNIX98/sys/sem.h/conform = yes
test-xfail-UNIX98/sys/shm.h/conform = yes
test-xfail-UNIX98/sys/socket.h/conform = yes
test-xfail-UNIX98/sys/time.h/conform = yes
test-xfail-UNIX98/sys/wait.h/conform = yes
test-xfail-UNIX98/ucontext.h/conform = yes
test-xfail-UNIX98/unistd.h/conform = yes
test-xfail-UNIX98/utmpx.h/conform = yes
test-xfail-XOPEN2K/aio.h/conform = yes
test-xfail-XOPEN2K/arpa/inet.h/conform = yes
test-xfail-XOPEN2K/fcntl.h/conform = yes
test-xfail-XOPEN2K/langinfo.h/conform = yes
test-xfail-XOPEN2K/math.h/conform = yes
test-xfail-XOPEN2K/mqueue.h/conform = yes
test-xfail-XOPEN2K/netdb.h/conform = yes
test-xfail-XOPEN2K/netinet/in.h/conform = yes
test-xfail-XOPEN2K/semaphore.h/conform = yes
test-xfail-XOPEN2K/signal.h/conform = yes
test-xfail-XOPEN2K/stdarg.h/conform = yes
test-xfail-XOPEN2K/stdio.h/conform = yes
test-xfail-XOPEN2K/stropts.h/conform = yes
test-xfail-XOPEN2K/sys/ipc.h/conform = yes
test-xfail-XOPEN2K/sys/msg.h/conform = yes
test-xfail-XOPEN2K/sys/sem.h/conform = yes
test-xfail-XOPEN2K/sys/shm.h/conform = yes
test-xfail-XOPEN2K/sys/socket.h/conform = yes
test-xfail-XOPEN2K/sys/wait.h/conform = yes
test-xfail-XOPEN2K/termios.h/conform = yes
test-xfail-XOPEN2K/tgmath.h/conform = yes
test-xfail-XOPEN2K/ucontext.h/conform = yes
test-xfail-XOPEN2K/utmpx.h/conform = yes
test-xfail-POSIX2008/arpa/inet.h/conform = yes
test-xfail-POSIX2008/fcntl.h/conform = yes
test-xfail-POSIX2008/netdb.h/conform = yes
test-xfail-POSIX2008/netinet/in.h/conform = yes
test-xfail-POSIX2008/signal.h/conform = yes
test-xfail-POSIX2008/stropts.h/conform = yes
test-xfail-POSIX2008/sys/socket.h/conform = yes
test-xfail-POSIX2008/sys/wait.h/conform = yes
test-xfail-XOPEN2K8/arpa/inet.h/conform = yes
test-xfail-XOPEN2K8/fcntl.h/conform = yes
test-xfail-XOPEN2K8/limits.h/conform = yes
test-xfail-XOPEN2K8/math.h/conform = yes
test-xfail-XOPEN2K8/netdb.h/conform = yes
test-xfail-XOPEN2K8/netinet/in.h/conform = yes
test-xfail-XOPEN2K8/signal.h/conform = yes
test-xfail-XOPEN2K8/stdio.h/conform = yes
test-xfail-XOPEN2K8/stropts.h/conform = yes
test-xfail-XOPEN2K8/sys/ipc.h/conform = yes
test-xfail-XOPEN2K8/sys/msg.h/conform = yes
test-xfail-XOPEN2K8/sys/select.h/conform = yes
test-xfail-XOPEN2K8/sys/sem.h/conform = yes
test-xfail-XOPEN2K8/sys/shm.h/conform = yes
test-xfail-XOPEN2K8/sys/socket.h/conform = yes
test-xfail-XOPEN2K8/sys/time.h/conform = yes
test-xfail-XOPEN2K8/sys/wait.h/conform = yes
test-xfail-XOPEN2K8/termios.h/conform = yes
test-xfail-XOPEN2K8/tgmath.h/conform = yes
test-xfail-XOPEN2K8/utmpx.h/conform = yes

conformtest-cc-flags = -I../include $(+sysdep-includes) $(sysincludes) -I..
$(conformtest-header-tests): $(objpfx)%/conform.out: \
			     conformtest.pl $(conformtest-headers-data)
	(set -e; std_hdr=$*; std=$${std_hdr%%/*}; hdr=$${std_hdr#*/}; \
	 mkdir -p $(@D)/scratch; \
	 $(PERL) conformtest.pl --tmpdir=$(@D)/scratch --cc='$(CC)' \
		 --flags='$(conformtest-cc-flags)' --standard=$$std \
		 --headers=$$hdr > $@); \
	$(evaluate-test)

$(linknamespace-symlists-tests): $(objpfx)symlist-%: list-header-symbols.pl
	$(PERL) -w $< --tmpdir=$(objpfx) --cc='$(CC)' \
		--flags='$(conformtest-cc-flags)' --standard=$* \
		--headers="$(strip $(conformtest-headers-$*))" \
		> $@ 2> $@.err; \
	$(evaluate-test)

linknamespace-libs-isoc = $(common-objpfx)libc.a $(common-objpfx)math/libm.a
linknamespace-libs = $(linknamespace-libs-isoc) \
		     $(common-objpfx)rt/librt.a $(static-thread-library)
linknamespace-libs-ISO = $(linknamespace-libs-isoc)
linknamespace-libs-ISO99 = $(linknamespace-libs-isoc)
linknamespace-libs-ISO11 = $(linknamespace-libs-isoc)
linknamespace-libs-XPG3 = $(linknamespace-libs-isoc)
linknamespace-libs-XPG4 = $(linknamespace-libs-isoc)
linknamespace-libs-POSIX = $(linknamespace-libs)
linknamespace-libs-UNIX98 = $(linknamespace-libs)
linknamespace-libs-XOPEN2K = $(linknamespace-libs)
linknamespace-libs-POSIX2008 = $(linknamespace-libs)
linknamespace-libs-XOPEN2K8 = $(linknamespace-libs)

$(linknamespace-symlist-stdlibs-tests): $(objpfx)symlist-stdlibs-%: \
					$(linknamespace-libs)
	LC_ALL=C $(READELF) -W -s $(linknamespace-libs-$*) > $@; \
	$(evaluate-test)

$(linknamespace-header-tests): $(objpfx)%/linknamespace.out: \
			       linknamespace.pl \
			       $(linknamespace-symlist-stdlibs-tests)
	(set -e; std_hdr=$*; std=$${std_hdr%%/*}; hdr=$${std_hdr#*/}; \
	 mkdir -p $(@D)/scratch; \
	 $(PERL) -w $< --tmpdir=$(@D)/scratch --cc='$(CC)' \
		 --flags='$(conformtest-cc-flags)' --standard=$$std \
		 --stdsyms=$(objpfx)symlist-$$std --header=$$hdr \
		 --libsyms=$(objpfx)symlist-stdlibs-$$std \
		 --readelf='$(READELF)' \
		 > $@ 2>&1); \
	$(evaluate-test)

# Pre-standard C feature no longer supported by GCC (obsoleted in
# newer POSIX standards).
test-xfail-XPG3/varargs.h/linknamespace = yes
test-xfail-XPG4/varargs.h/linknamespace = yes
test-xfail-UNIX98/varargs.h/linknamespace = yes

# Header not provided by glibc.
test-xfail-XPG4/ndbm.h/linknamespace = yes
test-xfail-UNIX98/ndbm.h/linknamespace = yes
test-xfail-XOPEN2K/ndbm.h/linknamespace = yes
test-xfail-XOPEN2K8/ndbm.h/linknamespace = yes

# Unsorted expected failures.
test-xfail-XPG3/fnmatch.h/linknamespace = yes
test-xfail-XPG3/glob.h/linknamespace = yes
test-xfail-XPG3/regex.h/linknamespace = yes
test-xfail-XPG3/search.h/linknamespace = yes
test-xfail-XPG3/stdio.h/linknamespace = yes
test-xfail-XPG3/unistd.h/linknamespace = yes
test-xfail-XPG3/wordexp.h/linknamespace = yes
test-xfail-XPG4/fmtmsg.h/linknamespace = yes
test-xfail-XPG4/fnmatch.h/linknamespace = yes
test-xfail-XPG4/glob.h/linknamespace = yes
test-xfail-XPG4/netdb.h/linknamespace = yes
test-xfail-XPG4/regex.h/linknamespace = yes
test-xfail-XPG4/search.h/linknamespace = yes
test-xfail-XPG4/stdio.h/linknamespace = yes
test-xfail-XPG4/stdlib.h/linknamespace = yes
test-xfail-XPG4/syslog.h/linknamespace = yes
test-xfail-XPG4/unistd.h/linknamespace = yes
test-xfail-XPG4/wordexp.h/linknamespace = yes
test-xfail-POSIX/aio.h/linknamespace = yes
test-xfail-POSIX/fnmatch.h/linknamespace = yes
test-xfail-POSIX/glob.h/linknamespace = yes
test-xfail-POSIX/mqueue.h/linknamespace = yes
test-xfail-POSIX/regex.h/linknamespace = yes
test-xfail-POSIX/semaphore.h/linknamespace = yes
test-xfail-POSIX/unistd.h/linknamespace = yes
test-xfail-POSIX/wordexp.h/linknamespace = yes
test-xfail-UNIX98/fmtmsg.h/linknamespace = yes
test-xfail-UNIX98/fnmatch.h/linknamespace = yes
test-xfail-UNIX98/glob.h/linknamespace = yes
test-xfail-UNIX98/mqueue.h/linknamespace = yes
test-xfail-UNIX98/netdb.h/linknamespace = yes
test-xfail-UNIX98/regex.h/linknamespace = yes
test-xfail-UNIX98/search.h/linknamespace = yes
test-xfail-UNIX98/stdio.h/linknamespace = yes
test-xfail-UNIX98/stdlib.h/linknamespace = yes
test-xfail-UNIX98/syslog.h/linknamespace = yes
test-xfail-UNIX98/unistd.h/linknamespace = yes
test-xfail-UNIX98/wchar.h/linknamespace = yes
test-xfail-UNIX98/wordexp.h/linknamespace = yes
test-xfail-XOPEN2K/fmtmsg.h/linknamespace = yes
test-xfail-XOPEN2K/fnmatch.h/linknamespace = yes
test-xfail-XOPEN2K/glob.h/linknamespace = yes
test-xfail-XOPEN2K/netdb.h/linknamespace = yes
test-xfail-XOPEN2K/regex.h/linknamespace = yes
test-xfail-XOPEN2K/search.h/linknamespace = yes
test-xfail-XOPEN2K/signal.h/linknamespace = yes
test-xfail-XOPEN2K/spawn.h/linknamespace = yes
test-xfail-XOPEN2K/stdlib.h/linknamespace = yes
test-xfail-XOPEN2K/sys/wait.h/linknamespace = yes
test-xfail-XOPEN2K/syslog.h/linknamespace = yes
test-xfail-XOPEN2K/ucontext.h/linknamespace = yes
test-xfail-XOPEN2K/unistd.h/linknamespace = yes
test-xfail-XOPEN2K/wordexp.h/linknamespace = yes
test-xfail-POSIX2008/dirent.h/linknamespace = yes
test-xfail-POSIX2008/grp.h/linknamespace = yes
test-xfail-POSIX2008/netdb.h/linknamespace = yes
test-xfail-POSIX2008/regex.h/linknamespace = yes
test-xfail-POSIX2008/semaphore.h/linknamespace = yes
test-xfail-POSIX2008/spawn.h/linknamespace = yes
test-xfail-POSIX2008/unistd.h/linknamespace = yes
test-xfail-XOPEN2K8/dirent.h/linknamespace = yes
test-xfail-XOPEN2K8/fmtmsg.h/linknamespace = yes
test-xfail-XOPEN2K8/grp.h/linknamespace = yes
test-xfail-XOPEN2K8/netdb.h/linknamespace = yes
test-xfail-XOPEN2K8/pwd.h/linknamespace = yes
test-xfail-XOPEN2K8/regex.h/linknamespace = yes
test-xfail-XOPEN2K8/search.h/linknamespace = yes
test-xfail-XOPEN2K8/spawn.h/linknamespace = yes
test-xfail-XOPEN2K8/syslog.h/linknamespace = yes
test-xfail-XOPEN2K8/unistd.h/linknamespace = yes
