ifeq ($(subdir),misc)
sysdep_headers += sys/rse.h
endif

ifeq ($(subdir),stdlib)
sysdep_routines += __start_context
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
