abi-variants := 32 64
abi-32-options := -U__s390x__
abi-32-condition := __WORDSIZE == 32
abi-64-options := -D__s390x__
abi-64-condition := __WORDSIZE == 64

ifeq ($(subdir),rt)
librt-routines += rt-sysdep
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
endif

ifeq ($(subdir),nptl)
ifeq ($(enable-lock-elision),yes)
libpthread-sysdep_routines += elision-lock elision-unlock elision-timed \
			      elision-trylock

elision-CFLAGS = -mhtm
CFLAGS-elision-lock.c = $(elision-CFLAGS)
CFLAGS-elision-timed.c = $(elision-CFLAGS)
CFLAGS-elision-trylock.c = $(elision-CFLAGS)
CFLAGS-elision-unlock.c = $(elision-CFLAGS)
endif
endif
