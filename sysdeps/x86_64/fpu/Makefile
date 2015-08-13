ifeq ($(subdir),mathvec)
libmvec-support += svml_d_cos2_core svml_d_cos4_core_avx \
		   svml_d_cos4_core svml_d_cos8_core \
		   svml_d_sin2_core svml_d_sin4_core_avx \
		   svml_d_sin4_core svml_d_sin8_core svml_d_trig_data \
		   svml_s_cosf4_core svml_s_cosf8_core_avx \
		   svml_s_cosf8_core svml_s_cosf16_core svml_s_trig_data \
		   svml_s_sinf4_core svml_s_sinf8_core_avx \
		   svml_s_sinf8_core svml_s_sinf16_core \
		   svml_d_sincos2_core svml_d_sincos4_core_avx \
		   svml_d_sincos4_core svml_d_sincos8_core \
		   svml_d_log2_core svml_d_log4_core_avx svml_d_log4_core \
		   svml_d_log8_core svml_d_log_data svml_s_logf4_core \
		   svml_s_logf8_core_avx svml_s_logf8_core svml_s_logf16_core \
		   svml_s_logf_data svml_d_exp2_core svml_d_exp4_core_avx \
		   svml_d_exp4_core svml_d_exp8_core svml_d_exp_data \
		   svml_s_expf4_core svml_s_expf8_core_avx svml_s_expf8_core \
		   svml_s_expf16_core svml_s_expf_data svml_d_pow2_core \
		   svml_d_pow4_core_avx svml_d_pow4_core svml_d_pow8_core \
		   svml_d_pow_data svml_s_powf4_core svml_s_powf8_core_avx \
		   svml_s_powf8_core svml_s_powf16_core svml_s_powf_data \
		   svml_s_sincosf4_core svml_s_sincosf8_core_avx \
		   svml_s_sincosf8_core svml_s_sincosf16_core
endif

# Variables for libmvec tests.
ifeq ($(subdir),math)
ifeq ($(build-mathvec),yes)
libmvec-tests += double-vlen2 double-vlen4 double-vlen4-avx2 \
		 float-vlen4 float-vlen8 float-vlen8-avx2

ifeq (yes,$(config-cflags-avx512))
libmvec-tests += double-vlen8 float-vlen16
endif

double-vlen4-arch-ext-cflags = -mavx
double-vlen4-arch-ext2-cflags = -mavx2
double-vlen8-arch-ext-cflags = -mavx512f

float-vlen8-arch-ext-cflags = -mavx
float-vlen8-arch-ext2-cflags = -mavx2
float-vlen16-arch-ext-cflags = -mavx512f

CFLAGS-test-double-vlen4-avx2.c = $(libm-test-vec-cflags)
CFLAGS-test-double-vlen4-avx2-wrappers.c = $(double-vlen4-arch-ext2-cflags)

CFLAGS-test-float-vlen8-avx2.c = $(libm-test-vec-cflags)
CFLAGS-test-float-vlen8-avx2-wrappers.c = $(float-vlen8-arch-ext2-cflags)

endif
endif
