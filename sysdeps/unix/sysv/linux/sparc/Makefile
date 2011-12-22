syscall-list-variants := 32bit 64bit
syscall-list-32bit-options := -U__sparc_v9__ -U__arch64__
syscall-list-32bit-condition := __WORDSIZE == 32
syscall-list-64bit-options := -D__sparc_v9__ -D__arch64__
syscall-list-64bit-condition := __WORDSIZE == 64

ifeq ($(subdir),rt)
librt-routines += rt-sysdep
endif
