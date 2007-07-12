ifeq ($(filter nptl,$(sorted-subdirs)),nptl)

ifeq ($(subdir),rtkaio)
librtkaio-sysdep_routines += kaio_timer_routines librtkaio-cancellation rtkaio-unwind-resume
librtkaio-shared-only-routines += rtkaio-unwind-resume
tests += tst-cancel17 tst-cancelx17
CPPFLAGS-kaio_timer_routines.c = -I../nptl
CFLAGS-librtkaio-cancellation.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-rtkaio-unwind-resume.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-tst-cancelx17.c += -fexceptions

ifeq (yes,$(build-shared))
$(objpfx)tst-timer: $(objpfx)librtkaio.so $(shared-thread-library)
else
$(objpfx)tst-timer: $(objpfx)librtkaio.a $(static-thread-library)
endif

ifeq ($(have-forced-unwind),yes)
tests += tst-mqueue8x
CFLAGS-tst-mqueue8x.c += -fexceptions
endif
endif

endif

ifeq ($(filter linuxthreads,$(sorted-subdirs)),linuxthreads)

ifeq ($(subdir),rtkaio)
librtkaio-sysdep_routines += kaio_timer_routines
CPPFLAGS += -DBROKEN_THREAD_SIGNALS

ifeq (yes,$(build-shared))
$(objpfx)tst-timer: $(objpfx)librtkaio.so $(shared-thread-library)
else
$(objpfx)tst-timer: $(objpfx)librtkaio.a $(static-thread-library)
endif
endif

endif
