ifeq ($(subdir),misc)
sysdep_routines += ioperm
sysdep_headers += sys/elf.h sys/io.h
endif

ifeq ($(subdir),signal)
sysdep_routines += sigrestorer
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif
