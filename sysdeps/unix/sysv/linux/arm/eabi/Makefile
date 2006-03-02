# Set this flag here so that arm/nptl/Makefile will see it.
arm-using-eabi = yes

ifeq ($(subdir),csu)
# In order for unwinding to fail when it falls out of main, we need a
# cantunwind marker.  There's one in start.S.  To make sure we reach it, add
# unwind tables for __libc_start_main.
CFLAGS-libc-start.c += -fexceptions
endif
