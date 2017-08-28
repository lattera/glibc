abi-variants := 32 64-v1 64-v2
abi-32-condition := __WORDSIZE == 32
abi-64-v1-condition := __WORDSIZE == 64 && _CALL_ELF != 2
abi-64-v2-condition := __WORDSIZE == 64 && _CALL_ELF == 2

ifeq ($(subdir),rt)
librt-routines += rt-sysdep
librt-shared-only-routines += rt-sysdep
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
ifeq ($(build-shared),yes)
# This is needed for DSO loading from static binaries.
sysdep-dl-routines += dl-static
endif
# Otherwise tst-tls-dlinfo fails due to tst-tlsmod2.so using static tls.
LDFLAGS-tst-tlsmod2.so += -Wl,--no-tls-get-addr-optimize
endif

ifeq ($(subdir),misc)
sysdep_headers += bits/ppc.h
sysdep_routines += get_timebase_freq
tests += test-gettimebasefreq
tests += test-powerpc-linux-sysconf
endif

ifeq ($(subdir),nptl)
libpthread-routines += sysdep
libpthread-sysdep_routines += elision-lock elision-unlock elision-timed \
			      elision-trylock
libpthread-shared-only-routines += sysdep
endif
