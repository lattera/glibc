ifeq ($(subdir),math)
libm-sysdep_routines += e_expf-sse2 s_sinf-sse2 s_cosf-sse2 \
                        s_sincosf-sse2

CFLAGS-e_expf-sse2.c = -msse2 -mfpmath=sse
endif
