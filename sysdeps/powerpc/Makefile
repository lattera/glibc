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
# get offset to rtld_global._dl_hwcap and rtld_global._dl_hwcap2
gen-as-const-headers += rtld-global-offsets.sym
# get offset to __locale_struct.__ctype_tolower
gen-as-const-headers += locale-defines.sym
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/platform/ppc.h
tests += test-gettimebase
endif

ifneq (,$(filter %le,$(config-machine)))
abilist-pattern = %-le.abilist
endif
