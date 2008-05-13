# The i387 `long double' is a distinct type we support.
long-double-fcts = yes

ifeq ($(subdir),csu)
sysdep_routines += hp-timing
elide-routines.os += hp-timing
endif

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),string)
sysdep_routines += cacheinfo
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += tlsdesc dl-tlsdesc
sysdep_routines += tlsdesc dl-tlsdesc
sysdep-rtld-routines += tlsdesc dl-tlsdesc
endif

ifeq ($(subdir),csu)
gen-as-const-headers += tlsdesc.sym
endif
