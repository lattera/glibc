ifeq ($(subdir),csu)
tests += test-multiarch
endif

ifeq ($(subdir),string)

sysdep_routines += strncat-c stpncpy-c strncpy-c \
		   strcmp-sse2 strcmp-sse2-unaligned strcmp-ssse3  \
		   strcmp-sse4_2 \
		   strncmp-sse2 strncmp-ssse3 strncmp-sse4_2 \
		   memchr-sse2 rawmemchr-sse2 memchr-avx2 rawmemchr-avx2 \
		   memrchr-sse2 memrchr-avx2 \
		   memcmp-sse2 \
		   memcmp-avx2-movbe \
		   memcmp-sse4 memcpy-ssse3 \
		   memmove-ssse3 \
		   memcpy-ssse3-back \
		   memmove-ssse3-back \
		   memmove-avx512-no-vzeroupper \
		   strcasecmp_l-sse2 strcasecmp_l-ssse3 \
		   strcasecmp_l-sse4_2 strcasecmp_l-avx \
		   strncase_l-sse2 strncase_l-ssse3 \
		   strncase_l-sse4_2 strncase_l-avx \
		   strchr-sse2 strchrnul-sse2 strchr-avx2 strchrnul-avx2 \
		   strrchr-sse2 strrchr-avx2 \
		   strlen-sse2 strnlen-sse2 strlen-avx2 strnlen-avx2 \
		   strcat-ssse3 strncat-ssse3\
		   strcpy-sse2 stpcpy-sse2 \
		   strcpy-ssse3 strncpy-ssse3 stpcpy-ssse3 stpncpy-ssse3 \
		   strcpy-sse2-unaligned strncpy-sse2-unaligned \
		   stpcpy-sse2-unaligned stpncpy-sse2-unaligned \
		   strcat-sse2 \
		   strcat-sse2-unaligned strncat-sse2-unaligned \
		   strchr-sse2-no-bsf memcmp-ssse3 strstr-sse2-unaligned \
		   strcspn-sse2 strpbrk-sse2 strspn-sse2 \
		   strcspn-c strpbrk-c strspn-c varshift \
		   memset-avx512-no-vzeroupper \
		   memmove-sse2-unaligned-erms \
		   memmove-avx-unaligned-erms \
		   memmove-avx512-unaligned-erms \
		   memset-sse2-unaligned-erms \
		   memset-avx2-unaligned-erms \
		   memset-avx512-unaligned-erms
CFLAGS-varshift.c += -msse4
CFLAGS-strcspn-c.c += -msse4
CFLAGS-strpbrk-c.c += -msse4
CFLAGS-strspn-c.c += -msse4
endif

ifeq ($(subdir),wcsmbs)
sysdep_routines += wmemcmp-sse4 wmemcmp-ssse3 wmemcmp-c \
		   wmemcmp-avx2-movbe \
		   wmemchr-sse2 wmemchr-avx2 \
		   wcscpy-ssse3 wcscpy-c \
		   wcschr-sse2 wcschr-avx2 \
		   wcsrchr-sse2 wcsrchr-avx2 \
		   wcsnlen-sse4_1 wcsnlen-c \
		   wcslen-sse2 wcslen-avx2 wcsnlen-avx2
endif

ifeq ($(subdir),debug)
sysdep_routines += memcpy_chk-nonshared mempcpy_chk-nonshared \
		   memmove_chk-nonshared memset_chk-nonshared \
		   wmemset_chk-nonshared
endif
