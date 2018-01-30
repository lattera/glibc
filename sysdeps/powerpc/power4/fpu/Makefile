# Makefile fragment for POWER4/5/5+ with FPU.

ifeq ($(subdir),math)
CFLAGS-mpa.c += --param max-unroll-times=4 -funroll-loops -fpeel-loops
endif
