ifeq ($(subdir),linuxthreads)
# On i686 we must avoid generating the trampoline functions generated
# to get the GOT pointer.
CFLAGS-pt-initfini.s += -march=i386 -mcpu=i386
endif
