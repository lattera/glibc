ifeq ($(subdir),misc)
sysdep_headers += sys/rse.h
endif

ifeq ($(subdir),stdlib)
sysdep_routines += __start_context
gen-as-const-headers += sigcontext-offsets.sym
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/io.h
sysdep_routines += ioperm clone2
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += dl-static
sysdep_routines += $(sysdep-dl-routines)
sysdep-rtld-routines += $(sysdep-dl-routines)
endif

ifeq ($(subdir),rt)
librt-routines += rt-sysdep
endif

# This is a crude attempt to silence the compiler which complains about
# then 'current' definition in the kernel headers.
CPPFLAGS += -D_ASM_IA64_CURRENT_H
