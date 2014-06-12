ifeq ($(subdir),csu)
gen-as-const-headers += tcb-offsets.sym
endif

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),debug)
CFLAGS-backtrace.c += -funwind-tables
endif
