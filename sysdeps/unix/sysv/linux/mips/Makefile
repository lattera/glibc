ifeq ($(subdir),signal)
#sysdep_routines += sigsuspend
endif

ifeq ($(subdir),misc)
sysdep_routines += cachectl cacheflush sysmips

headers += regdef.h fpregdef.h sys/asm.h sys/cachectl.h sys/fpregdef.h \
	   sys/regdef.h sys/sysmips.h
endif
