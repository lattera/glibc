# pull in __syscall_error routine
libpthread-routines += sysdep

ifeq ($(subdir),nptl)
# Avoid .cfi_startproc/endproc markers when creating init and fini pieces.
CFLAGS-pt-initfini.s += -fno-asynchronous-unwind-tables
endif
