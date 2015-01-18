ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),misc)
# MIPS/Tile-style cacheflush routine
sysdep_headers += sys/cachectl.h
sysdep_routines += cacheflush
endif

ifeq ($(subdir),nptl)
libpthread-routines := $(filter-out pt-vfork,$(libpthread-routines))
endif
