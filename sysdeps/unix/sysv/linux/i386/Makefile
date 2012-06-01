# The default ABI is 32.
default-abi := 32

ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86 call_pselect6 call_fallocate
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
