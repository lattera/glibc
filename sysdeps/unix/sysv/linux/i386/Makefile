ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86 s_pread64 s_pwrite64
sysdep_headers += sys/perm.h sys/vm86.h
endif

ifeq ($(subdir),elf)
others      += lddlibc4
install-bin += lddlibc4
endif
