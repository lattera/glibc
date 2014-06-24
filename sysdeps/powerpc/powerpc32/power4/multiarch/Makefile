ifeq ($(subdir),string)
sysdep_routines += memcpy-power7 memcpy-a2 memcpy-power6 memcpy-cell \
		   memcpy-ppc32 memcmp-power7 memcmp-ppc32 memset-power7 \
		   memset-power6 memset-ppc32 bzero-power7 bzero-power6 \
		   bzero-ppc32 mempcpy-power7 mempcpy-ppc32 memchr-power7 \
		   memchr-ppc32 memrchr-power7 memrchr-ppc32 rawmemchr-power7 \
		   rawmemchr-ppc32 strlen-power7 strlen-ppc32 strnlen-power7 \
		   strnlen-ppc32 strncmp-power7 strncmp-ppc32 \
		   strcasecmp-power7 strcasecmp_l-power7 strncase-power7 \
		   strncase_l-power7 strchrnul-power7 strchrnul-ppc32 \
		   strchr-power7 strchr-ppc32 wcschr-power7 wcschr-power6 \
		   wcschr-ppc32 wcsrchr-power7 wcsrchr-power6 wcsrchr-ppc32 \
		   wcscpy-power7 wcscpy-power6 wcscpy-ppc32 wordcopy-power7 \
		   wordcopy-power6 wordcopy-ppc32 memmove-power7 memmove-ppc

CFLAGS-strncase-power7.c += -mcpu=power7 -funroll-loops
CFLAGS-strncase_l-power7.c += -mcpu=power7 -funroll-loops
CFLAGS-wcschr-power7.c += -mcpu=power7
CFLAGS-wcschr-power6.c += -mcpu=power6
CFLAGS-wcsrchr-power7.c += -mcpu=power7
CFLAGS-wcsrchr-power6.c += -mcpu=power6
CFLAGS-wcscpy-power7.c += -mcpu=power7
CFLAGS-wcscpy-power6.c += -mcpu=power6
endif
