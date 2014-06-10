ifeq ($(subdir),misc)

# <sys/reg.h> provides something like x86 compatibility.
# New code should probably use <arch/abi.h> instead.
sysdep_headers += sys/reg.h

# MIPS-style cacheflush routine
sysdep_headers += sys/cachectl.h
sysdep_routines += cacheflush

# Control dataplane properties of current thread.
sysdep_headers += sys/dataplane.h
sysdep_routines += set_dataplane

endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
ifeq ($(build-shared),yes)
# This is needed for DSO loading from static binaries.
sysdep-dl-routines += dl-static
sysdep_routines += dl-static
sysdep-rtld-routines += dl-static
endif
endif

ifeq ($(subdir),nptl)
# pull in __syscall_error routine
libpthread-routines += sysdep
endif
