ifeq ($(subdir),linuxthreads)
# On i686 we must avoid generating the trampoline functions generated
# to get the GOT pointer.
CFLAGS-pt-initfini.s += -march=i386 -mcpu=i386

# Most files must not be compiled without frame pointer since we need
# the frame base address which is stored in %ebp unless the frame pointer
# is optimized out.
CFLAGS-cancel.c += -fno-omit-frame-pointer
CFLAGS-condvar.c += -fno-omit-frame-pointer
CFLAGS-join.c += -fno-omit-frame-pointer
CFLAGS-manager.c += -fno-omit-frame-pointer
CFLAGS-oldsemaphore.c += -fno-omit-frame-pointer
CFLAGS-pthreads.c += -fno-omit-frame-pointer
CFLAGS-ptlongjmp.c += -fno-omit-frame-pointer
CFLAGS-semaphore.c += -fno-omit-frame-pointer
CFLAGS-signals.c += -fno-omit-frame-pointer
endif
