ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),debug)
CFLAGS-backtrace.c += -funwind-tables
endif
