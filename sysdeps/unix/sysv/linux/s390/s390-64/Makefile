ifeq ($(subdir),csu)
CFLAGS-initfini.s += -DWEAK_GMON_START
endif

ifeq ($(subdir),misc)
sysdep_routines += setfsgid setfsuid setresgid setresuid
sysdep_headers += sys/elf.h
endif
