ifeq ($(subdir),rt)
librt-sysdep_routines += timer_routines
CPPFLAGS += -DBROKEN_THREAD_SIGNALS

ifeq (yes,$(build-shared))
$(objpfx)tst-timer: $(objpfx)librt.so $(shared-thread-library)
else
$(objpfx)tst-timer: $(objpfx)librt.a $(static-thread-library)
endif
endif

ifeq ($(subdir),posix)
CFLAGS-confstr.c += -DLIBPTHREAD_VERSION="\"$(shell sed 's/\(.*\) by .*/\1/' ../linuxthreads/Banner)\""
endif
