ifeq ($(subdir),math)
libm-sysdep_routines += s_floor-c s_ceil-c s_floorf-c s_ceilf-c \
			s_rint-c s_rintf-c s_nearbyint-c s_nearbyintf-c

ifeq ($(have-mfma4),yes)
libm-sysdep_routines += e_exp-fma4 e_log-fma4 e_pow-fma4 s_atan-fma4 \
			e_asin-fma4 e_atan2-fma4 s_sin-fma4 s_tan-fma4 \
			mplog-fma4 mpa-fma4 slowexp-fma4 slowpow-fma4 \
			sincos32-fma4 doasin-fma4 dosincos-fma4 \
			halfulp-fma4 mpexp-fma4 \
			mpatan2-fma4 mpatan-fma4 mpsqrt-fma4 mptan-fma4

CFLAGS-doasin-fma4.c = -mfma4
CFLAGS-dosincos-fma4.c = -mfma4
CFLAGS-e_asin-fma4.c = -mfma4
CFLAGS-e_atan2-fma4.c = -mfma4
CFLAGS-e_exp-fma4.c = -mfma4
CFLAGS-e_log-fma4.c = -mfma4
CFLAGS-e_pow-fma4.c = -mfma4
CFLAGS-halfulp-fma4.c = -mfma4
CFLAGS-mpa-fma4.c = -mfma4
CFLAGS-mpatan-fma4.c = -mfma4
CFLAGS-mpatan2-fma4.c = -mfma4
CFLAGS-mpexp-fma4.c = -mfma4
CFLAGS-mplog-fma4.c = -mfma4
CFLAGS-mpsqrt-fma4.c = -mfma4
CFLAGS-mptan-fma4.c = -mfma4
CFLAGS-s_atan-fma4.c = -mfma4
CFLAGS-sincos32-fma4.c = -mfma4
CFLAGS-slowexp-fma4.c = -mfma4
CFLAGS-slowpow-fma4.c = -mfma4
CFLAGS-s_sin-fma4.c = -mfma4
CFLAGS-s_tan-fma4.c = -mfma4
endif

ifeq ($(config-cflags-sse2avx),yes)
libm-sysdep_routines += e_exp-avx e_log-avx s_atan-avx \
			e_atan2-avx s_sin-avx s_tan-avx \
			mplog-avx mpa-avx slowexp-avx \
			mpexp-avx

CFLAGS-e_atan2-avx.c = -msse2avx -DSSE2AVX
CFLAGS-e_exp-avx.c = -msse2avx -DSSE2AVX
CFLAGS-e_log-avx.c = -msse2avx -DSSE2AVX
CFLAGS-mpa-avx.c = -msse2avx -DSSE2AVX
CFLAGS-mpexp-avx.c = -msse2avx -DSSE2AVX
CFLAGS-mplog-avx.c = -msse2avx -DSSE2AVX
CFLAGS-s_atan-avx.c = -msse2avx -DSSE2AVX
CFLAGS-s_sin-avx.c = -msse2avx -DSSE2AVX
CFLAGS-slowexp-avx.c = -msse2avx -DSSE2AVX
CFLAGS-s_tan-avx.c = -msse2avx -DSSE2AVX
endif
endif
