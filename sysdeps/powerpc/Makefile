# We always want to use the new mnemonic syntax even if we are on a RS6000
# machine.
+cflags += -mnew-mnemonics

ifeq ($(subdir),string)
CFLAGS-memcmp.c += -Wno-uninitialized
endif

ifeq ($(subdir),elf)
# extra shared linker files to link into dl-allobjs.so and libc
sysdep-dl-routines += dl-machine
sysdep_routines += dl-machine
# extra shared linker files to link only into dl-allobjs.so
sysdep-rtld-routines += dl-machine
endif

ifeq ($(subdir),setjmp)
ifeq (yes,$(build-shared))
sysdep_routines += novmx-longjmp novmx-sigjmp
endif
endif

ifeq ($(subdir),csu)
# get offset to rtld_global._dl_hwcap
gen-as-const-headers += rtld-global-offsets.sym
endif
