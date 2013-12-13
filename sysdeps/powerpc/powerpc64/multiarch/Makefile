ifeq ($(subdir),string)
sysdep_routines += memcpy-power7 memcpy-a2 memcpy-power6 memcpy-cell \
		   memcpy-power4 memcpy-ppc64 memcmp-power7 memcmp-power4 \
		   memcmp-ppc64 memset-power7 memset-power6 memset-power4 \
		   memset-ppc64 bzero-power4 bzero-power6 bzero-power7 \
		   mempcpy-power7 mempcpy-ppc64 memchr-power7 memchr-ppc64 \
		   memrchr-power7 memrchr-ppc64 rawmemchr-power7 \
		   rawmemchr-ppc64 strlen-power7 strlen-ppc64 strnlen-power7 \
		   strnlen-ppc64 strcasecmp-power7 strcasecmp_l-power7 \
		   strncase-power7 strncase_l-power7 strncmp-power7 \
		   strncmp-power4 strncmp-ppc64 strchr-power7 strchr-ppc64 \
		   strchrnul-power7 strchrnul-ppc64

CFLAGS-strncase-power7.c += -mcpu=power7 -funroll-loops
CFLAGS-strncase_l-power7.c += -mcpu=power7 -funroll-loops
endif
