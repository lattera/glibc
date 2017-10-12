ifeq ($(subdir),math)
libm-sysdep_routines += m_copysignf-vis3 m_copysign-vis3 s_fabs-vis3 \
			s_fabsf-vis3 s_llrintf-vis3 s_llrint-vis3 \
			s_rintf-vis3 s_rint-vis3 \
			s_fmaf-vis3 s_fma-vis3 s_fma-generic s_fmaf-generic \
			s_nearbyint-vis3 s_nearbyintf-vis3 \
			s_fdimf-vis3 s_fdim-vis3 s_fdim-generic \
			s_fdimf-generic
sysdep_routines += s_copysignf-vis3 s_copysign-vis3

CFLAGS-s_fdimf-vis3.c += -Wa,-Av9d -mvis3
CFLAGS-s_fdim-vis3.c += -Wa,-Av9d -mvis3
endif
