ifeq ($(subdir),csu)
sysdep_routines += hp-timing
elide-routines.os += hp-timing
endif

# So that we can test __m128's alignment
stack-align-test-flags += -msse
