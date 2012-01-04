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
CFLAGS-initfini.s += -fno-unwind-tables
endif

ifeq ($(subdir),nptl)
CFLAGS-pt-initfini.s += -fno-unwind-tables
endif

ASFLAGS-.os += $(pic-ccflag)
