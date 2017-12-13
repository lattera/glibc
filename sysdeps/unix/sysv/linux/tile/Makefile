# Provide biarch definitions.
abi-variants := 64 32
abi-64-condition := __WORDSIZE == 64
abi-32-condition := __WORDSIZE == 32

ifeq ($(subdir),misc)

# <sys/reg.h> provides something like x86 compatibility.
# New code should probably use <arch/abi.h> instead.
sysdep_headers += sys/reg.h

# MIPS-style cacheflush routine
sysdep_headers += sys/cachectl.h
sysdep_routines += cacheflush

# Control dataplane properties of current thread (compatibility only).
sysdep_routines += set_dataplane

endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
ifeq ($(build-shared),yes)
# This is needed for DSO loading from static binaries.
sysdep-dl-routines += dl-static
endif
endif

ifeq ($(subdir),nptl)
# pull in __syscall_error routine
libpthread-routines += sysdep
libpthread-shared-only-routines += sysdep
endif
