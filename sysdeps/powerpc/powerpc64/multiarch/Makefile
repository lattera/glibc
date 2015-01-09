ifeq ($(subdir),string)
sysdep_routines += memcpy-power7 memcpy-a2 memcpy-power6 memcpy-cell \
		   memcpy-power4 memcpy-ppc64 memcmp-power7 memcmp-power4 \
		   memcmp-ppc64 memset-power7 memset-power6 memset-power4 \
		   memset-ppc64 memset-power8 \
		   mempcpy-power7 mempcpy-ppc64 memchr-power7 memchr-ppc64 \
		   memrchr-power7 memrchr-ppc64 rawmemchr-power7 \
		   rawmemchr-ppc64 strlen-power7 strlen-ppc64 strnlen-power7 \
		   strnlen-ppc64 strcasecmp-power7 strcasecmp_l-power7 \
		   strncase-power7 strncase_l-power7 \
		   strncmp-power8 strncmp-power7 strncmp-power4 strncmp-ppc64 \
		   strchr-power7 strchr-ppc64 \
		   strchrnul-power7 strchrnul-ppc64 wcschr-power7 \
		   wcschr-power6 wcschr-ppc64 wcsrchr-power7 wcsrchr-power6 \
		   wcsrchr-ppc64 wcscpy-power7 wcscpy-power6 wcscpy-ppc64 \
		   wordcopy-power7 wordcopy-power6 wordcopy-ppc64 \
		   strcpy-power8 strcpy-power7 strcpy-ppc64 stpcpy-power8 \
		   stpcpy-power7 stpcpy-ppc64 \
		   strrchr-power7 strrchr-ppc64 strncat-power7 strncat-ppc64 \
		   strncpy-power7 strncpy-ppc64 \
		   stpncpy-power8 stpncpy-power7 stpncpy-ppc64 \
		   strcmp-power8 strcmp-power7 strcmp-ppc64 \
		   strcat-power8 strcat-power7 strcat-ppc64 memmove-power7 \
		   memmove-ppc64 bcopy-ppc64 strncpy-power8

CFLAGS-strncase-power7.c += -mcpu=power7 -funroll-loops
CFLAGS-strncase_l-power7.c += -mcpu=power7 -funroll-loops
CFLAGS-wcschr-power7.c += -mcpu=power7
CFLAGS-wcschr-power6.c += -mcpu=power6
CFLAGS-wcsrchr-power7.c += -mcpu=power7
CFLAGS-wcsrchr-power6.c += -mcpu=power6
CFLAGS-wcscpy-power7.c += -mcpu=power7
CFLAGS-wcscpy-power6.c += -mcpu=power6
CFLAGS-wordcopy-power7.c += -mcpu=power7
CFLAGS-wordcopy-power6.c += -mcpu=power6
endif
