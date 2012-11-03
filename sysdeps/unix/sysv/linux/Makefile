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

ifeq ($(subdir),socket)
sysdep_routines += internal_accept4 internal_recvmmsg internal_sendmmsg
endif

ifeq ($(subdir),misc)
include $(firstword $(wildcard $(sysdirs:=/sysctl.mk)))

sysdep_routines += clone llseek umount umount2 readahead \
		   setfsuid setfsgid makedev epoll_pwait signalfd \
		   eventfd eventfd_read eventfd_write prlimit

CFLAGS-gethostid.c = -fexceptions
CFLAGS-tst-writev.c += "-DARTIFICIAL_LIMIT=0x80000000-__getpagesize()"

sysdep_headers += sys/mount.h sys/acct.h sys/sysctl.h \
		  sys/klog.h sys/kdaemon.h \
		  sys/user.h sys/procfs.h sys/prctl.h \
		  sys/kd.h sys/soundcard.h sys/vt.h \
		  sys/quota.h sys/fsuid.h \
		  scsi/sg.h scsi/scsi.h scsi/scsi_ioctl.h sys/pci.h \
		  sys/ultrasound.h sys/raw.h sys/personality.h sys/epoll.h \
		  bits/a.out.h sys/inotify.h sys/signalfd.h sys/eventfd.h \
		  sys/timerfd.h sys/fanotify.h bits/eventfd.h bits/inotify.h \
		  bits/signalfd.h bits/timerfd.h bits/epoll.h \
		  bits/socket_type.h bits/syscall.h bits/sysctl.h

tests += tst-clone

# Generate the list of SYS_* macros for the system calls (__NR_* macros).

# If there is more than one syscall list for different architecture
# variants, the CPU/Makefile defines abi-variants to be a list of names
# for those variants (e.g. 32 64), and, for each variant, defines
# abi-$(variant)-options to be compiler options to cause <asm/unistd.h>
# to define the desired list of syscalls and abi-$(variant)-condition to
# be the condition for those options to use in a C #if condition.
# abi-includes may be defined to a list of headers to include
# in the generated header, if the default does not suffice.

$(objpfx)bits/syscall%h $(objpfx)bits/syscall%d: ../sysdeps/unix/sysv/linux/sys/syscall.h
	$(make-target-directory)
	{ \
	 echo '/* Generated at libc build time from kernel syscall list.  */';\
	 echo ''; \
	 echo '#ifndef _SYSCALL_H'; \
	 echo '# error "Never use <bits/syscall.h> directly; include <sys/syscall.h> instead."'; \
	 echo '#endif'; \
	 echo ''; \
	 $(foreach h,$(abi-includes), echo '#include <$(h)>';) \
	 echo ''; \
	 $(if $(abi-variants), \
	 $(foreach v,$(abi-variants),\
	 $(CC) -E -MD -MP -MF $(@:.h=.d)-t$(v) -MT '$(@:.d=.h) $(@:.h=.d)' \
	       -x c $(sysincludes) $< $(abi-$(v)-options) \
	       -D_LIBC -dM | \
	 sed -n 's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p' | \
	 LC_ALL=C sort > $(@:.d=.h).new$(v); \
	 $(if $(abi-$(v)-condition),\
	 echo '#if $(abi-$(v)-condition)';) \
	 cat $(@:.d=.h).new$(v); \
	 $(if $(abi-$(v)-condition),echo '#endif';) \
	 rm -f $(@:.d=.h).new$(v); \
	 ), \
	 $(CC) -E -MD -MP -MF $(@:.h=.d)-t$(v) -MT '$(@:.d=.h) $(@:.h=.d)' \
	       -x c $(sysincludes) $< \
	       -D_LIBC -dM | \
	 sed -n 's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p' | \
	 LC_ALL=C sort;) \
	} > $(@:.d=.h).new
	mv -f $(@:.d=.h).new $(@:.d=.h)
ifdef abi-variants
ifneq (,$(objpfx))
	sed $(sed-remove-objpfx) \
	 $(foreach v,$(abi-variants),$(@:.h=.d)-t$(v)) > $(@:.h=.d)-t3
else
	cat $(foreach v,$(abi-variants),$(@:.h=.d)-t$(v)) \
	 > $(@:.h=.d)-t3
endif
	rm -f $(foreach v,$(abi-variants),$(@:.h=.d)-t$(v))
	mv -f $(@:.h=.d)-t3 $(@:.h=.d)
else
	mv -f $(@:.h=.d)-t $(@:.h=.d)
endif

ifndef no_deps
# Get the generated list of dependencies (probably /usr/include/asm/unistd.h).
-include $(objpfx)bits/syscall.d
endif
generated += bits/syscall.h bits/syscall.d
endif

ifeq ($(subdir),time)
sysdep_headers += sys/timex.h bits/timex.h

sysdep_routines += ntp_gettime ntp_gettimex
endif

ifeq ($(subdir),socket)
sysdep_headers += net/if_ppp.h net/ppp-comp.h \
		  net/ppp_defs.h net/if_arp.h net/route.h net/ethernet.h \
		  net/if_slip.h net/if_packet.h net/if_shaper.h
sysdep_routines += cmsg_nxthdr sa_len
endif

ifeq ($(subdir),sunrpc)
sysdep_headers += nfs/nfs.h
endif

ifeq ($(subdir),termios)
sysdep_headers += termio.h
endif

ifeq ($(subdir),posix)
sysdep_headers += bits/initspin.h

sysdep_routines += exit-thread sched_getcpu

tests += tst-getcpu
endif

ifeq ($(subdir),inet)
sysdep_headers += netinet/if_fddi.h netinet/if_tr.h \
		  netipx/ipx.h netash/ash.h netax25/ax25.h netatalk/at.h \
		  netrom/netrom.h netpacket/packet.h netrose/rose.h \
		  neteconet/ec.h netiucv/iucv.h
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
		   sync_file_range open_2 open64_2 fallocate fallocate64
sysdep_headers += bits/fcntl-linux.h
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += dl-brk dl-sbrk dl-getcwd dl-openat64 dl-opendir \
			dl-fxstatat64

CPPFLAGS-lddlibc4 += -DNOT_IN_libc
endif

ifeq ($(subdir),rt)
CFLAGS-mq_send.c += -fexceptions
CFLAGS-mq_receive.c += -fexceptions
endif

ifeq ($(subdir),nscd)
sysdep-CFLAGS += -DHAVE_EPOLL -DHAVE_SENDFILE -DHAVE_INOTIFY -DHAVE_NETLINK
CFLAGS-gai.c += -DNEED_NETLINK
endif
