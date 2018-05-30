ifeq ($(subdir),math)
libm-sysdep_routines += w_sqrtf128-power9 w_sqrtf128-ppc64le

CFLAGS-w_sqrtf128-ppc64le.c += -mfloat128
CFLAGS-w_sqrtf128-power9.c += -mfloat128 -mcpu=power9
endif
