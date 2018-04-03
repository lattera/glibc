ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),misc)
# MIPS-style cacheflush routine
sysdep_headers += sys/cachectl.h
sysdep_routines += cacheflush
endif
