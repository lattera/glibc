ifeq ($(subdir),posix)
sysdep_routines += oldglob
endif

ifeq ($(subdir),misc)
sysdep_headers += alpha/ptrace.h alpha/regdef.h

sysdep_routines += ieee_get_fp_control ieee_set_fp_control \
		   sethae ioperm osf_sigprocmask llseek adjtimex

# Support old timeval32 entry points
sysdep_routines += osf_select osf_gettimeofday osf_settimeofday \
		   osf_getitimer osf_setitimer osf_utimes \
		   osf_getrusage osf_wait4 old_adjtimex

CFLAGS-ioperm.c = -Wa,-mev6
endif
