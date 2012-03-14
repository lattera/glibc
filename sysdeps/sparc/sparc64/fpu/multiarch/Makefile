ifeq ($(subdir),math)
ifeq ($(have-as-vis3),yes)
libm-sysdep_routines += m_signbitf-vis3 m_signbit-vis3
sysdep_routines += s_signbitf-vis3 s_signbit-vis3

CFLAGS-s_signbitf-vis3.S = -Wa,-Av9d
CFLAGS-s_signbit-vis3.S = -Wa,-Av9d
endif
endif