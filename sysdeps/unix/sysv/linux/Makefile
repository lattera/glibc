ifeq ($(subdir),csu)
CFLAGS-init-first.c = -fkeep-inline-functions

sysdep_routines += errno-loc
endif

ifeq ($(subdir),misc)
sysdep_routines += sysctl clone llseek

sysdep_headers += sys/mount.h sys/acct.h sys/sysctl.h sys/mtio.h \
		  sys/module.h sys/io.h sys/klog.h sys/kdaemon.h \
		  sys/user.h syscall-list.h sys/sysmacros.h sys/procfs.h \
		  sys/debugreg.h sys/kd.h sys/soundcard.h sys/vt.h

# Generate the list of SYS_* macros for the system calls (__NR_* macros).
$(objpfx)syscall-%.h $(objpfx)syscall-%.d: ../sysdeps/unix/sysv/linux/sys/syscall.h
	rm -f $(@:.h=.d)
	echo > $(@:.d=.h).new \
	     '/* Generated at libc build time from kernel syscall list.  */'
	SUNPRO_DEPENDENCIES='$(@:.h=.d) $(patsubst $(objpfx)%,$$(objpfx)%,\
			     $(@:.h=.d))' \
	$(CC) -E -x c $< -D_LIBC -dM | \
	sed -n >> $(@:.d=.h).new \
	      's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p'
	mv -f $(@:.d=.h).new $(@:.d=.h)
# Get the generated list of dependencies (probably /usr/include/asm/unistd.h).
-include $(objpfx)syscall-list.d
generated += syscall-list.h syscall-list.d
endif

ifeq ($(subdir),time)
sysdep_headers += sys/timex.h
endif

ifeq ($(subdir),socket)
sysdep_headers += sys/socketcall.h net/if.h net/if_ppp.h net/ppp-comp.h \
		  net/ppp_defs.h net/if_arp.h net/route.h
sysdep_routines += cmsg_nxthdr
endif

ifeq ($(subdir),sunrpc)
sysdep_headers += nfs/nfs.h
endif

ifeq ($(subdir),termios)
sysdep_headers += termio.h sys/kernel_termios.h
endif

ifeq ($(subdir),stdio-common)
# Just disable the auto generation in sysdeps/generic
inhibit-siglist := yes
endif

ifeq ($(subdir),inet)
sysdep_headers += netinet/in_systm.h sys/socketvar.h
endif

# Don't compile the ctype glue code, since there is no old non-GNU C library.
inhibit-glue = yes
