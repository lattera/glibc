ifeq ($(subdir),math)
sysdep_routines += s_isnan-power7 s_isnan-power6x s_isnan-power6 \
		   s_isnan-power5 s_isnan-ppc64 s_copysign-power6 \
		   s_copysign-ppc64 s_finite-power7 s_finite-ppc64 \
		   s_finitef-ppc64 s_isinff-ppc64 s_isinf-power7 \
		   s_isinf-ppc64 s_modf-power5+ s_modf-ppc64 \
		   s_modff-power5+ s_modff-ppc64 s_isnan-power8 \
		   s_isinf-power8 s_finite-power8

libm-sysdep_routines += s_isnan-power7 s_isnan-power6x s_isnan-power6 \
			s_isnan-power5 s_isnan-ppc64 s_llround-power6x \
			s_llround-power5+ s_llround-ppc64 s_ceil-power5+ \
			s_ceil-ppc64 s_ceilf-power5+ s_ceilf-ppc64 \
			s_floor-power5+ s_floor-ppc64 s_floorf-power5+ \
			s_floorf-ppc64 s_round-power5+ s_round-ppc64 \
			s_roundf-power5+ s_roundf-ppc64 s_trunc-power5+ \
			s_trunc-ppc64 s_truncf-power5+ s_truncf-ppc64 \
			s_copysign-power6 s_copysign-ppc64 s_llrint-power6x \
			s_llrint-ppc64 s_finite-power7 s_finite-ppc64 \
			s_finitef-ppc64 s_isinff-ppc64 s_isinf-power7 \
			s_isinf-ppc64 s_logb-power7 s_logbf-power7 \
			s_logbl-power7 s_logb-ppc64 s_logbf-ppc64 \
			s_logbl-ppc64 s_modf-power5+ s_modf-ppc64 \
			s_modff-power5+ s_modff-ppc64 e_hypot-ppc64 \
			e_hypot-power7 e_hypotf-ppc64 e_hypotf-power7 \
			s_isnan-power8 s_isinf-power8 s_finite-power8 \
			s_llrint-power8 s_llround-power8

CFLAGS-s_logbf-power7.c = -mcpu=power7
CFLAGS-s_logbl-power7.c = -mcpu=power7
CFLAGS-s_logb-power7.c = -mcpu=power7
CFLAGS-s_modf-power5+.c = -mcpu=power5+
CFLAGS-s_modff-power5+.c = -mcpu=power5+
CFLAGS-e_hypot-power7.c = -mcpu=power7
CFLAGS-e_hypotf-power7.c = -mcpu=power7
endif
