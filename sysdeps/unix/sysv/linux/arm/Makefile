ifeq ($(subdir),misc)
sysdep_routines += ioperm
sysdep_headers += sys/elf.h sys/io.h
endif

ifeq ($(subdir),signal)
sysdep_routines += rt_sigsuspend rt_sigprocmask rt_sigtimedwait	\
		   rt_sigqueueinfo rt_sigaction rt_sigpending \
		   sigrestorer
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif
