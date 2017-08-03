ifeq ($(subdir),string)
CFLAGS-memcmp.c += -Wno-uninitialized
endif

ifeq ($(subdir),elf)
# extra shared linker files to link into dl-allobjs.so and libc
sysdep-dl-routines += dl-machine hwcapinfo
sysdep_routines += dl-machine hwcapinfo
# extra shared linker files to link only into dl-allobjs.so
sysdep-rtld-routines += dl-machine hwcapinfo

modules-names += mod-tlsopt-powerpc
mod-tlsopt-powerpc.so-no-z-defs = yes
tests += tst-tlsopt-powerpc
$(objpfx)tst-tlsopt-powerpc: $(objpfx)mod-tlsopt-powerpc.so

ifneq (no,$(multi-arch))
tests-static += tst-tlsifunc-static
tests-internal += tst-tlsifunc-static
ifeq (yes,$(build-shared))
tests-internal += tst-tlsifunc
endif # build-shared
endif # multi-arch
endif # subdir = elf

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

ifeq ($(subdir),nptl)
tests-internal += test-get_hwcap test-get_hwcap-static
tests-static += test-get_hwcap-static
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/platform/ppc.h
tests += test-gettimebase
tests += tst-set_ppr
endif

ifneq (,$(filter %le,$(config-machine)))
abilist-pattern = %-le.abilist
endif
