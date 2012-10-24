abi-variants := 32 64
abi-32-options := -U__powerpc64__
abi-32-condition := __WORDSIZE == 32
abi-64-options := -D__powerpc64__
abi-64-condition := __WORDSIZE == 64
abi-64-ld-soname := ld64.so.1

ifeq ($(subdir),rt)
librt-routines += rt-sysdep
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
endif

ifeq ($(subdir),misc)
sysdep_headers += bits/ppc.h
sysdep_routines += get_timebase_freq
tests += test-gettimebasefreq
endif
