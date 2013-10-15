ifeq ($(subdir),math)
sysdep_routines += s_isnan-power7 s_isnan-power6 s_isnan-power5 s_isnan-ppc32 \
		   s_isnanf-power6 s_isnanf-power5 s_isinf-power7 \
		   s_isinf-ppc32 s_isinff-ppc32 s_finite-power7 \
		   s_finite-ppc32 s_finitef-ppc32 s_copysign-power6 \
		   s_copysign-ppc32 s_modf-power5+ s_modf-ppc32 \
		   s_modff-power5+ s_modff-ppc32

libm-sysdep_routines += s_llrintf-power6 s_llrintf-ppc32 s_llrint-power6 \
			s_llrint-ppc32 s_llround-power6 s_llround-power5+ \
			s_llround-ppc32 w_sqrt-power5 w_sqrt-ppc32 \
			w_sqrtf-power5 w_sqrtf-ppc32 s_isnan-power7 \
			s_isnan-power6 s_isnan-power5 s_isnan-ppc32 \
			s_isnanf-power6 s_isnanf-power5 s_isinf-power7 \
			s_isinf-ppc32 s_isinff-ppc32 s_finite-power7 \
			s_finite-ppc32 s_finitef-ppc32 s_ceil-power5+ \
			s_ceil-ppc32 s_ceilf-power5+ s_ceilf-ppc32 \
			s_floor-power5+ s_floor-ppc32 s_floorf-power5+ \
			s_floorf-ppc32 s_round-power5+ s_round-ppc32 \
			s_roundf-power5+ s_roundf-ppc32 s_trunc-power5+ \
			s_trunc-ppc32 s_truncf-power5+ s_truncf-ppc32 \
			s_copysign-power6 s_copysign-ppc32 s_lround-power6x \
			s_lround-power5+ s_lround-ppc32 s_lrint-power6x \
			s_lrint-ppc32 s_modf-power5+ s_modf-ppc32 \
			s_modff-power5+ s_modff-ppc32 s_logbl-power7 \
			s_logbl-ppc32 s_logb-power7 s_logb-ppc32 \
			s_logbf-power7 s_logbf-ppc32 e_hypot-power7 \
			e_hypot-ppc32 e_hypotf-power7 e_hypotf-ppc32

CFLAGS-s_modf-power5+.c = -mcpu=power5+
CFLAGS-s_modff-power5+.c = -mcpu=power5+
CFLAGS-s_logbl-power7.c = -mcpu=power7
CFLAGS-s_logb-power7.c = -mcpu=power7
CFLAGS-s_logbf-power7.c = -mcpu=power7
CFLAGS-e_hypot-power7.c = -mcpu=power7
CFLAGS-e_hypotf-power7.c = -mcpu=power7
endif
