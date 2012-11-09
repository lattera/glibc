long-double-fcts = yes

ifeq ($(subdir),elf)
sysdep-dl-routines += tlsdesc dl-tlsdesc
sysdep_routines += tlsdesc dl-tlsdesc
sysdep-rtld-routines += tlsdesc dl-tlsdesc
gen-as-const-headers += dl-link.sym
endif

ifeq ($(subdir),csu)
gen-as-const-headers += tlsdesc.sym
endif
