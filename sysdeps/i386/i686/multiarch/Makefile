ifeq ($(subdir),csu)
aux += init-arch
gen-as-const-headers += ifunc-defines.sym
endif

ifeq ($(subdir),string)
sysdep_routines += bzero-sse2 memset-sse2 memcpy-ssse3 mempcpy-ssse3 \
		   memmove-ssse3 memcpy-ssse3-rep mempcpy-ssse3-rep \
		   memmove-ssse3-rep bcopy-ssse3 bcopy-ssse3-rep \
		   memset-sse2-rep bzero-sse2-rep
ifeq (yes,$(config-cflags-sse4))
sysdep_routines += strcspn-c strpbrk-c strspn-c strstr-c strcasestr-c
CFLAGS-strcspn-c.c += -msse4
CFLAGS-strpbrk-c.c += -msse4
CFLAGS-strspn-c.c += -msse4
CFLAGS-strstr.c += -msse4
CFLAGS-strcasestr.c += -msse4
endif
endif
