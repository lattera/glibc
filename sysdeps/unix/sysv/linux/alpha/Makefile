ifeq ($(subdir),posix)
sysdep_routines += oldglob
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext-offsets.sym
endif

ifeq ($(subdir),misc)
sysdep_headers += alpha/ptrace.h alpha/regdef.h sys/io.h

sysdep_routines += ieee_get_fp_control ieee_set_fp_control \
		   ioperm llseek

# Support old timeval32 entry points
sysdep_routines += osf_select osf_gettimeofday osf_settimeofday \
		   osf_getitimer osf_setitimer osf_utimes \
		   osf_getrusage osf_wait4

# Support old ipc control
sysdep_routines += oldmsgctl oldsemctl oldshmctl

CFLAGS-ioperm.c = -Wa,-mev6
endif

ifeq ($(subdir),signal)
sysdep_routines += rt_sigaction
endif
