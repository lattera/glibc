syscall-list-variants := 32bit 64bit
syscall-list-32bit-options := -U__s390x__
syscall-list-32bit-condition := __WORDSIZE == 32
syscall-list-64bit-options := -D__s390x__
syscall-list-64bit-condition := __WORDSIZE == 64

ifeq ($(subdir),rt)
librt-routines += rt-sysdep
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
endif
