ifeq ($(subdir),csu)
sysdep_routines      += __read_tp libc-__read_tp
static-only-routines += __read_tp
shared-only-routines += libc-__read_tp
endif

ifeq ($(subdir),elf)
sysdep_routines	     += dl-vdso
sysdep-rtld-routines += __read_tp
ifeq ($(build-shared),yes)
# This is needed for DSO loading from static binaries.
sysdep-dl-routines += dl-static
sysdep_routines += dl-static
sysdep-rtld-routines += dl-static
endif
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/elf.h
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif
