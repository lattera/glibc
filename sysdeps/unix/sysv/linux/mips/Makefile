ifeq ($(subdir),signal)
sysdep_routines += rt_sigsuspend rt_sigprocmask rt_sigtimedwait	\
		   rt_sigqueueinfo rt_sigaction rt_sigpending
#sysdep_routines += sigsuspend
endif

ifeq ($(subdir),misc)
sysdep_routines += cachectl cacheflush sysmips

headers += regdef.h fpregdef.h sys/asm.h sys/cachectl.h sys/fpregdef.h \
	   sys/regdef.h sys/sysmips.h
endif
