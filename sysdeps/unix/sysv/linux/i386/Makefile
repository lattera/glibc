ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86 s_pread64 s_pwrite64
sysdep_headers += sys/elf.h sys/perm.h sys/reg.h sys/vm86.h
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4
endif

ifeq ($(subdir),csu)
CFLAGS-initfini.s += -DWEAK_GMON_START
endif
