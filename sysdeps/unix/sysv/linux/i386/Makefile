ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86 setfsgid setfsuid setresgid setresuid
sysdep_headers += sys/elf.h sys/perm.h sys/reg.h sys/vm86.h sys/debugreg.h sys/io.h
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4

ifeq (yes,$(build-shared))
# This is needed to support g++ v2 and v3.
sysdep_routines += framestate
shared-only-routines += framestate
endif
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif
