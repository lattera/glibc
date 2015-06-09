ifeq ($(subdir),mathvec)
libmvec-support += svml_d_cos2_core svml_d_cos4_core_avx \
		   svml_d_cos4_core svml_d_cos8_core \
		   svml_d_cos_data svml_s_cosf4_core svml_s_cosf8_core_avx \
		   svml_s_cosf8_core svml_s_cosf16_core svml_s_cosf_data \
		   init-arch
endif

# Variables for libmvec tests.
ifeq ($(subdir),math)
ifeq ($(build-mathvec),yes)
libmvec-tests += double-vlen2 double-vlen4 double-vlen4-avx2

ifeq (yes,$(config-cflags-avx512))
libmvec-tests += double-vlen8
endif

double-vlen2-arch-ext-cflags = -msse4
double-vlen4-arch-ext-cflags = -mavx
double-vlen4-arch-ext2-cflags = -mavx2
double-vlen8-arch-ext-cflags = -mavx512f

CFLAGS-test-double-vlen4-avx2.c = $(libm-test-vec-cflags)
CFLAGS-test-double-vlen4-avx2-wrappers.c = $(double-vlen4-arch-ext2-cflags)

endif
endif
