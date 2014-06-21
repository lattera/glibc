# See Makeconfig regarding the use of default-abi.
default-abi := 64

sysdep-CFLAGS += -fcall-used-g6
LD += -melf64_sparc

ifeq ($(subdir),stdlib)
sysdep_routines += __start_context
endif

ifeq ($(subdir),nptl)
CFLAGS-pause.c += -fexceptions
CFLAGS-sigsuspend.c += -fexceptions
endif
