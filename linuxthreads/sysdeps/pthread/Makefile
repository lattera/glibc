ifeq ($(subdir),rt)
librt-routines += timer_routines

ifeq ($(have-thread-library),yes)
tests += tst-timer
endif

ifeq (yes,$(build-shared))
$(objpfx)tst-timer: $(objpfx)librt.so $(shared-thread-library)
else
$(objpfx)tst-timer: $(objpfx)librt.a $(static-thread-library)
endif
endif
