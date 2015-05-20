# The default ABI is 32.
default-abi := 32

ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4
endif

# fallocate, posix_fallocate use six-argument inline syscalls.
ifeq ($(subdir),io)
sysdep_routines += libc-do-syscall
endif

# libpthread uses six-argument inline syscalls.
ifeq ($(subdir),nptl)
libpthread-sysdep_routines += libc-do-syscall
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif
