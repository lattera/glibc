ifeq ($(subdir),misc)
sysdep_routines += setfsgid setfsuid setresgid setresuid
sysdep_headers += sys/elf.h
endif

ifeq ($(subdir),csu)
CFLAGS-initfini.s += -DWEAK_GMON_START
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif
