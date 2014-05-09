ifeq ($(subdir),math)
libm-support += powl_helper
tests += test-fenv-sse
CFLAGS-test-fenv-sse.c += -msse2 -mfpmath=sse
endif
