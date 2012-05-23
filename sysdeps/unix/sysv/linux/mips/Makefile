ifeq ($(subdir),signal)
#sysdep_routines += sigsuspend
endif

ifeq ($(subdir),misc)
sysdep_routines += cachectl cacheflush sysmips _test_and_set

sysdep_headers += sys/cachectl.h sys/sysmips.h sys/tas.h

# _MIPS_SIM_ABI32 == 1, _MIPS_SIM_ABIN32 == 2, _MIPS_SIM_ABI64 == 3
syscall-list-variants := o32 n32 n64
syscall-list-includes := sgidefs.h
syscall-list-o32-options := -D_MIPS_SIM=1
syscall-list-o32-condition := _MIPS_SIM == _MIPS_SIM_ABI32
syscall-list-n32-options := -D_MIPS_SIM=2
syscall-list-n32-condition := _MIPS_SIM == _MIPS_SIM_ABIN32
syscall-list-n64-options := -D_MIPS_SIM=3
syscall-list-n64-condition := _MIPS_SIM == _MIPS_SIM_ABI64
endif

ifeq ($(subdir),elf)
ifeq ($(build-shared),yes)
# This is needed for DSO loading from static binaries.
sysdep-dl-routines += dl-static
sysdep_routines += dl-static
sysdep-rtld-routines += dl-static
endif
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif
