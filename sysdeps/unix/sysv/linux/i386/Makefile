ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86
sysdep_headers += sys/elf.h sys/perm.h sys/reg.h sys/vm86.h sys/debugreg.h sys/io.h
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4

endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif
