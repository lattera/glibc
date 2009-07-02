ifeq ($(subdir),csu)
aux += init-arch
gen-as-const-headers += ifunc-defines.sym
endif

ifeq ($(subdir),string)
sysdep_routines += stpncpy-c strncpy-c strncmp-c
endif
