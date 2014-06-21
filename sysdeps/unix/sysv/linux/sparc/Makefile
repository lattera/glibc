abi-variants := 32 64
abi-32-options := -U__sparc_v9__ -U__arch64__
abi-32-condition := __WORDSIZE == 32
abi-64-options := -D__sparc_v9__ -D__arch64__
abi-64-condition := __WORDSIZE == 64

ifeq ($(subdir),rt)
librt-routines += rt-sysdep
endif

ifeq ($(subdir),sysvipc)
sysdep_routines += getshmlba
endif

ifeq ($(subdir),nptl)
# pull in __syscall_error routine
libpthread-routines += sysdep
endif
