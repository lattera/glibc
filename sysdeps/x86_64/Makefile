# The i387 `long double' is a distinct type we support.
long-double-fcts = yes

ifeq ($(subdir),csu)
sysdep_routines += hp-timing
static-only-routines += hp-timing
endif

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif
