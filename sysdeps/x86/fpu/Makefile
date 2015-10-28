ifeq ($(subdir),math)
libm-support += powl_helper
tests += test-fenv-sse test-fenv-clear-sse
CFLAGS-test-fenv-sse.c += -msse2 -mfpmath=sse
CFLAGS-test-fenv-clear-sse.c += -msse2 -mfpmath=sse
endif
