ifeq ($(subdir),misc)
sysdep_routines += ioperm
sysdep_headers += sys/io.h
endif

ifeq ($(subdir),csu)
CFLAGS-init-first.c += -momit-leaf-frame-pointer
endif
