ifeq ($(subdir), math)
sysdep_routines += setfpucw
extra-objs += ieee-fpucw.o
headers += fpu_control.h
install-lib += libieee.a
non-lib.a += libieee.a

$(objpfx)libieee.a: $(objpfx)ieee-fpucw.o
	rm -f $@
	ln $< $@
endif

ifeq ($(subdir), io)
sysdep_routines += fxstat lxstat xstat
endif

ifeq ($(subdir), misc)
sysdep_routines += xmknod mount umount s_ptrace
headers += sys/mount.h
endif

ifeq ($(subdir), time)
sysdep_routines += adjtimex
headers += sys/timex.h
endif

ifeq ($(subdir), socket)
headers += sys/socketcall.h
endif

ifeq ($(subdir), sunrpc)
headers += nfs/nfs.h
endif

rtld-installed-name = ld-gnu.so.1

# Don't compile the ctype glue code, since there is no old non-GNU C library.
inhibit-glue = yes

