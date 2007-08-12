64bit-predefine = __powerpc64__
ifeq ($(subdir),rt)
librt-routines += rt-sysdep
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
endif
