ifeq ($(subdir), io)
sysdep_routines += fxstat lxstat xstat
endif

ifeq ($(subdir), misc)
sysdep_routines += mount umount xmknod s_ptrace s_sysctl sysctl llseek \
setfsgid setfsuid sysinfo uselib s_reboot

sysdep_headers += sys/mount.h sys/sysinfo.h sys/acct.h sys/sysctl.h \
		  sys/module.h sys/io.h sys/klog.h sys/kdaemon.h \
		  sys/user.h syscall-list.h

# Generate the list of SYS_* macros for the system calls (__NR_* macros).
$(objpfx)syscall-%.h $(objpfx)syscall-%.d: ../sysdeps/unix/sysv/linux/syscall.h
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

ifeq ($(subdir), time)
sysdep_routines += adjtimex
sysdep_headers += sys/timex.h
endif

ifeq ($(subdir), socket)
sysdep_headers += sys/socketcall.h net/if.h
endif

ifeq ($(subdir), sunrpc)
sysdep_headers += nfs/nfs.h
endif

ifeq ($(subdir), termios)
sysdep_headers += termio.h
endif

rtld-installed-name = ld-gnu.so.1

# Don't compile the ctype glue code, since there is no old non-GNU C library.
inhibit-glue = yes
