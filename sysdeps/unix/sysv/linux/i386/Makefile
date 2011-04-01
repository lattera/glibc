ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86 call_pselect6 call_fallocate
sysdep_headers += sys/elf.h sys/perm.h sys/reg.h sys/vm86.h sys/debugreg.h sys/io.h
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4
endif

ifeq ($(subdir),io)
sysdep_routines += call_sync_file_range
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif
