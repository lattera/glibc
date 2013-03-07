# Makefile fragment for POWER4/5/5+ with FPU.

ifeq ($(subdir),math)
CFLAGS-mpa.c += --param max-unroll-times=4 -funroll-loops -fpeel-loops
CPPFLAGS-slowpow.c += -DUSE_LONG_DOUBLE_FOR_MP=1
CPPFLAGS-slowexp.c += -DUSE_LONG_DOUBLE_FOR_MP=1
endif
