# So that we can test __m128's alignment
stack-align-test-flags += -msse

ifeq ($(subdir),string)
sysdep_routines += cacheinfo
endif

ifeq (yes,$(config-asflags-i686))
CFLAGS-.o += -Wa,-mtune=i686
CFLAGS-.os += -Wa,-mtune=i686
CFLAGS-.op += -Wa,-mtune=i686
CFLAGS-.og += -Wa,-mtune=i686
CFLAGS-.oS += -Wa,-mtune=i686

ASFLAGS-.o += -Wa,-mtune=i686
ASFLAGS-.os += -Wa,-mtune=i686
ASFLAGS-.op += -Wa,-mtune=i686
ASFLAGS-.og += -Wa,-mtune=i686
ASFLAGS-.oS += -Wa,-mtune=i686
endif

ifeq ($(subdir),stdlib)
# _Unwind_Backtrace from libgcc produces a segmentation fault if it was
# called within a context created by makecontext. See Bug 18635.
test-xfail-tst-makecontext = yes
endif
