ifeq ($(subdir),csu)
aux += init-arch
gen-as-const-headers += ifunc-defines.sym
endif

ifeq ($(subdir),string)
ifeq (yes,$(config-cflags-sse4))
sysdep_routines += strcspn-c strpbrk-c strspn-c strstr-c strcasestr-c
CFLAGS-strcspn-c.c += -msse4
CFLAGS-strpbrk-c.c += -msse4
CFLAGS-strspn-c.c += -msse4
CFLAGS-strstr.c += -msse4
CFLAGS-strcasestr.c += -msse4
endif
endif
