# See Makeconfig regarding the use of default-abi.
default-abi := 32

ifeq ($(subdir),login)
sysdep_routines += utmp32 utmpx32
libutil-routines += login32
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/elf.h
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif

ifeq ($(subdir),elf)
ifeq (yes,$(build-shared))
# This is needed to support g++ v2 and v3.
sysdep_routines += framestate
shared-only-routines += framestate
endif
endif

ifeq ($(subdir),stdlib)
sysdep_routines += __makecontext_ret
endif
