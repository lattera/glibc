libpthread-routines += cpu_relax

ifeq ($(subdir),nptl)
CFLAGS-pause.c += -fexceptions
CFLAGS-sigsuspend.c += -fexceptions
endif
