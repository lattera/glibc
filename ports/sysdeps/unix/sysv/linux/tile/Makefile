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
endif
