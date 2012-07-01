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

ifeq ($(subdir),debug)
CFLAGS-backtrace.c += -funwind-tables
endif

ifeq ($(subdir),csu)
CPPFLAGS-crti.S += $(pic-ccflag)
CPPFLAGS-crtn.S += $(pic-ccflag)
endif

ifeq ($(subdir),nptl)
CPPFLAGS-pt-crti.S += $(pic-ccflag)
CPPFLAGS-crtn.S += $(pic-ccflag)
endif

ASFLAGS-.os += $(pic-ccflag)
