ifeq ($(subdir),csu)
CFLAGS-initfini.s += -DWEAK_GMON_START
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/io.h
sysdep_routines += ioperm
endif
