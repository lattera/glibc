# Makefile fragment for PowerPC with no FPU.

ifeq ($(subdir),soft-fp)
sysdep_routines += $(gcc-single-routines) $(gcc-double-routines) sim-full
endif

ifeq ($(subdir),math)
libm-support += fenv_const fe_nomask
endif
