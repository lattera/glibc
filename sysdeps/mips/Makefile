ifeq ($(subdir),misc)
sysdep_headers += regdef.h fpregdef.h sys/regdef.h sys/fpregdef.h \
		  sys/asm.h sgidefs.h
endif

ifeq ($(subdir),setjmp)
sysdep_routines += setjmp_aux
endif

ifeq ($(subdir),rt)
librt-sysdep_routines += rt-sysdep
endif
