ifeq ($(subdir),csu)
CFLAGS-init-first.c = -fkeep-inline-functions

sysdep_routines += errno-loc
endif

ifeq ($(subdir),misc)
sysdep_routines += sysctl clone llseek

sysdep_headers += sys/mount.h sys/acct.h sys/sysctl.h sys/mtio.h \
		  sys/module.h sys/io.h sys/klog.h sys/kdaemon.h \
		  sys/user.h syscall-list.h sys/sysmacros.h sys/procfs.h \
		  sys/debugreg.h sys/kd.h sys/soundcard.h sys/vt.h \
		  sys/quota.h

# Generate the list of SYS_* macros for the system calls (__NR_* macros).
$(objpfx)syscall-%.h $(objpfx)syscall-%.d: ../sysdeps/unix/sysv/linux/sys/syscall.h
	rm -f $(@:.h=.d)
	echo > $(@:.d=.h).new \
	     '/* Generated at libc build time from kernel syscall list.  */'
	SUNPRO_DEPENDENCIES='$(@:.h=.d) $(patsubst $(objpfx)%,$$(objpfx)%,\
						   $(@:.d=.h) $(@:.h=.d))' \
	$(CC) -E -x c $< -D_LIBC -dM | \
	sed -n >> $(@:.d=.h).new \
	      's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p'
	mv -f $(@:.d=.h).new $(@:.d=.h)
ifndef no_deps
# Get the generated list of dependencies (probably /usr/include/asm/unistd.h).
-include $(objpfx)syscall-list.d
endif
generated += syscall-list.h syscall-list.d
endif

ifeq ($(subdir),time)
sysdep_headers += sys/timex.h
endif

ifeq ($(subdir),socket)
sysdep_headers += net/if.h net/if_ppp.h net/ppp-comp.h \
		  net/ppp_defs.h net/if_arp.h net/route.h net/ethernet.h \
		  net/if_slip.h
sysdep_routines += cmsg_nxthdr
endif

ifeq ($(subdir),sunrpc)
sysdep_headers += nfs/nfs.h
endif

ifeq ($(subdir),termios)
sysdep_headers += termio.h
endif

ifeq ($(subdir),stdio-common)
# Just disable the auto generation in sysdeps/generic
inhibit-siglist := yes
endif

ifeq ($(subdir),inet)
sysdep_headers += netinet/in_systm.h netinet/udp.h \
		  netinet/if_fddi.h netinet/if_tr.h netinet/igmp.h \
		  netinet/ip_fw.h  netinet/ip_icmp.h netipx/ipx.h \
		  sys/socketvar.h
endif

# Don't compile the ctype glue code, since there is no old non-GNU C library.
inhibit-glue = yes

# Tell sysdeps/posix/Makefile that we create stdio_lim.h differently.
inhibit-stdio_lim = yes

$(common-objpfx)stdio_%.h $(common-objpfx)stdio_%.d: \
 $(..)sysdeps/unix/sysv/linux/stdio_%.h.in
	rm -f $(@:.h=.d) $(@:.d=.h)-t
	echo '#include <linux/limits.h>' | \
	SUNPRO_DEPENDENCIES='$(@:.h=.d) \
			     $(patsubst $(common-objpfx)%,$$(common-objpfx)%,\
					$(@:.d=.h) $(@:.h=.d))' \
	$(CC) -E -dM - > $(@:.d=.h)-t
	fopen_max=`sed -n 's/^#define OPEN_MAX //p' $(@:.d=.h)-t`; \
	filename_max=`sed -n 's/^#define PATH_MAX //p' $(@:.d=.h)-t`; \
	if test -n "$$fopen_max" && test -n "$$filename_max"; then \
		sed -e "s/DEFAULT_FOPEN_MAX/$$fopen_max/" \
		    -e "s/DEFAULT_FILENAME_MAX/$$filename_max/" $< \
		    > $(@:.d=.h).new && \
		mv -f $(@:.d=.h).new $(@:.d=.h); \
	else exit 1; fi
# Remove this last so that it can be examined if something went wrong.
	rm -f $(@:.d=.h)-t
ifndef no_deps
# Get the list of dependencies (probably /usr/include/linux/limits.h).
-include $(common-objpfx)stdio_lim.d
endif
common-generated += stdio_lim.h stdio_lim.d
