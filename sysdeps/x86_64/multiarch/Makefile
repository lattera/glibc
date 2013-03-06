ifeq ($(subdir),csu)
aux += init-arch
tests += test-multiarch
gen-as-const-headers += ifunc-defines.sym
endif

ifeq ($(subdir),string)

sysdep_routines += strncat-c stpncpy-c strncpy-c strcmp-ssse3 strncmp-ssse3 \
		   strend-sse4 memcmp-sse4 memcpy-ssse3 mempcpy-ssse3 \
		   memmove-ssse3 memcpy-ssse3-back mempcpy-ssse3-back \
		   memmove-ssse3-back strcasestr-nonascii strcasecmp_l-ssse3 \
		   strncase_l-ssse3 strlen-sse4 strlen-sse2-no-bsf memset-x86-64 \
		   strcpy-ssse3 strncpy-ssse3 stpcpy-ssse3 stpncpy-ssse3 \
		   strcpy-sse2-unaligned strncpy-sse2-unaligned \
		   stpcpy-sse2-unaligned stpncpy-sse2-unaligned \
		   strcat-sse2-unaligned strncat-sse2-unaligned \
		   strcat-ssse3 strncat-ssse3 strlen-sse2-pminub \
		   strnlen-sse2-no-bsf strrchr-sse2-no-bsf strchr-sse2-no-bsf \
		   memcmp-ssse3
ifeq (yes,$(config-cflags-sse4))
sysdep_routines += strcspn-c strpbrk-c strspn-c strstr-c strcasestr-c varshift
CFLAGS-varshift.c += -msse4
CFLAGS-strcspn-c.c += -msse4
CFLAGS-strpbrk-c.c += -msse4
CFLAGS-strspn-c.c += -msse4
CFLAGS-strstr.c += -msse4
CFLAGS-strcasestr.c += -msse4
CFLAGS-strcasestr-nonascii.c += -msse4
endif
endif

ifeq ($(subdir),wcsmbs)
sysdep_routines += wmemcmp-sse4 wmemcmp-ssse3 wmemcmp-c wcscpy-ssse3 wcscpy-c
endif
