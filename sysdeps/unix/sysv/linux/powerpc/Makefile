abi-variants := 32 64-v1 64-v2
abi-32-options := -U__powerpc64__
abi-32-condition := __WORDSIZE == 32
abi-64-v1-options := -D__powerpc64__ -U_CALL_ELF -D_CALL_ELF=1
abi-64-v1-condition := __WORDSIZE == 64 && _CALL_ELF != 2
abi-64-v2-options := -D__powerpc64__ -U_CALL_ELF -D_CALL_ELF=2
abi-64-v2-condition := __WORDSIZE == 64 && _CALL_ELF == 2

ifeq ($(subdir),rt)
librt-routines += rt-sysdep
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
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

ifeq ($(subdir),misc)
sysdep_headers += bits/ppc.h
sysdep_routines += get_timebase_freq
tests += test-gettimebasefreq
endif

ifeq ($(subdir),nptl)
libpthread-routines += sysdep
libpthread-sysdep_routines += elision-lock elision-unlock elision-timed \
			      elision-trylock
endif
