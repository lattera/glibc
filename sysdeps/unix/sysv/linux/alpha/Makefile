ifeq ($(subdir),posix)
sysdep_routines += oldglob
endif

ifeq ($(subdir),misc)
sysdep_headers += alpha/ptrace.h alpha/regdef.h sys/io.h

sysdep_routines += ieee_get_fp_control ieee_set_fp_control \
		   ioperm osf_sigprocmask llseek adjtimex

# Support old timeval32 entry points
sysdep_routines += osf_select osf_gettimeofday osf_settimeofday \
		   osf_getitimer osf_setitimer osf_utimes \
		   osf_getrusage osf_wait4 old_adjtimex

# Support old ipc control
sysdep_routines += oldmsgctl oldsemctl oldshmctl

CFLAGS-ioperm.c = -Wa,-mev6
endif

ifeq ($(subdir),signal)
sysdep_routines += rt_sigsuspend rt_sigprocmask rt_sigtimedwait	\
		   rt_sigqueueinfo rt_sigaction rt_sigpending
endif

ifeq ($(subdir),elf)
ifeq (yes,$(build-shared))
# This is needed to support g++ v2 and v3.
sysdep_routines += framestate
shared-only-routines += framestate
endif
endif
