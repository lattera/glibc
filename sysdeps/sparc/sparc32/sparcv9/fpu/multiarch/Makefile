ifeq ($(subdir),math)
ifeq ($(have-as-vis3),yes)
libm-sysdep_routines += m_copysignf-vis3 m_copysign-vis3
sysdep_routines += s_copysignf-vis3 s_copysign-vis3

CFLAGS-s_copysignf-vis3.S = -Wa,-Av9d
CFLAGS-s_copysign-vis3.S = -Wa,-Av9d
endif
endif