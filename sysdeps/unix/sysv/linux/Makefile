ifeq ($(subdir),csu)
sysdep_routines += errno-loc
endif

ifeq ($(subdir),assert)
CFLAGS-assert.c += -DFATAL_PREPARE_INCLUDE='<fatal-prepare.h>'
CFLAGS-assert-perr.c += -DFATAL_PREPARE_INCLUDE='<fatal-prepare.h>'
endif

ifeq ($(subdir),malloc)
CFLAGS-malloc.c += -DMORECORE_CLEARS=2
endif

ifeq ($(subdir),misc)
include $(firstword $(wildcard $(sysdirs:=/sysctl.mk)))

sysdep_routines += clone umount umount2 readahead \
		   setfsuid setfsgid epoll_pwait signalfd \
		   eventfd eventfd_read eventfd_write prlimit \
		   personality epoll_wait tee vmsplice splice \
		   open_by_handle_at mlock2 pkey_mprotect pkey_set pkey_get

CFLAGS-gethostid.c = -fexceptions
CFLAGS-tee.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-vmsplice.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-splice.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-open_by_handle_at.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sync_file_range.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-tst-writev.c += "-DARTIFICIAL_LIMIT=(0x80000000-sysconf(_SC_PAGESIZE))"

sysdep_headers += sys/mount.h sys/acct.h sys/sysctl.h \
		  sys/klog.h \
		  sys/user.h sys/prctl.h \
		  sys/kd.h sys/soundcard.h sys/vt.h \
		  sys/quota.h sys/fsuid.h \
		  scsi/sg.h scsi/scsi.h scsi/scsi_ioctl.h sys/pci.h \
		  sys/raw.h sys/personality.h sys/epoll.h \
		  bits/a.out.h sys/inotify.h sys/signalfd.h sys/eventfd.h \
		  sys/timerfd.h sys/fanotify.h bits/eventfd.h bits/inotify.h \
		  bits/signalfd.h bits/timerfd.h bits/epoll.h \
		  bits/socket_type.h bits/syscall.h bits/sysctl.h \
		  bits/mman-linux.h bits/mman-shared.h bits/ptrace-shared.h \
		  bits/siginfo-arch.h bits/siginfo-consts-arch.h

tests += tst-clone tst-clone2 tst-clone3 tst-fanotify tst-personality \
	 tst-quota tst-sync_file_range tst-sysconf-iov_max tst-ttyname \
	 test-errno-linux tst-memfd_create tst-mlock2 tst-pkey \
	 tst-rlimit-infinity tst-ofdlocks
tests-internal += tst-ofdlocks-compat


# Generate the list of SYS_* macros for the system calls (__NR_*
# macros).  The file syscall-names.list contains all possible system
# call names, and the generated header file produces SYS_* macros for
# the __NR_* macros which are actually defined.

generated += bits/syscall.h
$(objpfx)bits/syscall.h: \
  ../sysdeps/unix/sysv/linux/gen-syscall-h.awk \
  ../sysdeps/unix/sysv/linux/syscall-names.list
	$(make-target-directory)
	LC_ALL=C $(AWK) -f $^ > $@-tmp
	$(move-if-change) $@-tmp $@
before-compile += $(objpfx)bits/syscall.h

# All macros defined by <sys/syscall.h>.  Include <bits/syscall.h>
# explicitly because <sys/sycall.h> skips it if _LIBC is defined.
$(objpfx)tst-syscall-list-macros.list: \
  $(objpfx)bits/syscall.h ../sysdeps/unix/sysv/linux/sys/syscall.h
	printf '#include <linux/version.h>\n\
#include <sys/syscall.h>\n#include <bits/syscall.h>\n' | \
	  $(CC) -E -o $@-tmp $(CFLAGS) $(CPPFLAGS) -x c - -dM
	$(move-if-change) $@-tmp $@

# __NR_* system call names.  Used by the test below.
$(objpfx)tst-syscall-list-nr.list: \
  ../sysdeps/unix/sysv/linux/filter-nr-syscalls.awk \
  $(objpfx)tst-syscall-list-macros.list
	LC_ALL=C $(AWK) -f $^ > $@-tmp
	$(move-if-change) $@-tmp $@

# SYS_* system call names.  Used by the test below.
$(objpfx)tst-syscall-list-sys.list: $(objpfx)tst-syscall-list-macros.list
	LC_ALL=C $(AWK) '/^#define SYS_/ { print substr($$2, 5) }' $< > $@-tmp
	$(move-if-change) $@-tmp $@

tests-special += $(objpfx)tst-syscall-list.out
$(objpfx)tst-syscall-list.out: \
  ../sysdeps/unix/sysv/linux/tst-syscall-list.sh \
  $(objpfx)tst-syscall-list-macros.list \
  $(objpfx)tst-syscall-list-nr.list \
  $(objpfx)tst-syscall-list-sys.list
	$(BASH) $^ $(AWK) > $@; $(evaluate-test)

# Separate object file for access to the constant from the UAPI header.
$(objpfx)tst-sysconf-iov_max: $(objpfx)tst-sysconf-iov_max-uapi.o

$(objpfx)tst-pkey: $(shared-thread-library)

endif # $(subdir) == misc

ifeq ($(subdir),time)
sysdep_headers += sys/timex.h bits/timex.h

sysdep_routines += ntp_gettime ntp_gettimex
endif

ifeq ($(subdir),signal)
tests-special += $(objpfx)tst-signal-numbers.out
# Depending on signal.o* is a hack.  What we actually want is a dependency
# on signal.h and everything it includes.  That's impractical to write
# in this context, but signal.c includes signal.h and not much else so it'll
# be conservatively correct.
$(objpfx)tst-signal-numbers.out: \
		../sysdeps/unix/sysv/linux/tst-signal-numbers.sh \
		$(objpfx)signal.o*
	AWK=$(AWK) $(SHELL) ../sysdeps/unix/sysv/linux/tst-signal-numbers.sh \
	$(CC) $(patsubst -DMODULE_NAME=%,-DMODULE_NAME=testsuite,$(CPPFLAGS)) \
	< /dev/null > $@; $(evaluate-test)
endif

ifeq ($(subdir),socket)
sysdep_headers += net/if_ppp.h net/ppp-comp.h \
		  net/ppp_defs.h net/if_arp.h net/route.h net/ethernet.h \
		  net/if_slip.h net/if_packet.h net/if_shaper.h
sysdep_routines += cmsg_nxthdr
CFLAGS-recvmmsg.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sendmmsg.c = -fexceptions -fasynchronous-unwind-tables
endif

ifeq ($(subdir),sunrpc)
sysdep_headers += nfs/nfs.h
endif

ifeq ($(subdir),termios)
sysdep_headers += termio.h
endif

ifeq ($(subdir),posix)
sysdep_headers += bits/initspin.h

sysdep_routines += sched_getcpu oldglob

tests += tst-affinity tst-affinity-pid

CFLAGS-fork.c = $(libio-mtsafe)
CFLAGS-getpid.o = -fomit-frame-pointer
CFLAGS-getpid.os = -fomit-frame-pointer
endif

ifeq ($(subdir),inet)
sysdep_headers += netinet/if_fddi.h netinet/if_tr.h \
		  netipx/ipx.h netash/ash.h netax25/ax25.h netatalk/at.h \
		  netrom/netrom.h netpacket/packet.h netrose/rose.h \
		  neteconet/ec.h netiucv/iucv.h
sysdep_routines += netlink_assert_response
endif

# Don't compile the ctype glue code, since there is no old non-GNU C library.
inhibit-glue = yes

ifeq ($(subdir),dirent)
sysdep_routines += getdirentries getdirentries64
endif

ifeq ($(subdir),nis)
CFLAGS-ypclnt.c = -DUSE_BINDINGDIR=1
endif

ifeq ($(subdir),io)
sysdep_routines += xstatconv internal_statvfs internal_statvfs64 \
		   sync_file_range fallocate fallocate64 \
		   close_nocancel fcntl_nocancel nanosleep_nocancel \
		   open_nocancel open64_nocancel \
		   openat_nocancel openat64_nocancel \
		   pause_nocancel read_nocancel waitpid_nocancel write_nocancel

sysdep_headers += bits/fcntl-linux.h

tests += tst-fallocate tst-fallocate64
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += dl-brk dl-sbrk dl-getcwd dl-openat64 dl-opendir \
			dl-fxstatat64

libof-lddlibc4 = lddlibc4

others += pldd
install-bin += pldd
$(objpfx)pldd: $(objpfx)xmalloc.o
endif

ifeq ($(subdir),rt)
CFLAGS-mq_send.c += -fexceptions
CFLAGS-mq_receive.c += -fexceptions
endif

ifeq ($(subdir),nscd)
sysdep-CFLAGS += -DHAVE_EPOLL -DHAVE_INOTIFY -DHAVE_NETLINK
CFLAGS-gai.c += -DNEED_NETLINK
endif

ifeq ($(subdir),nptl)
tests += tst-align-clone tst-getpid1 \
	tst-thread-affinity-pthread tst-thread-affinity-pthread2 \
	tst-thread-affinity-sched
tests-internal += tst-setgetname
endif
