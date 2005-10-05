ifeq ($(subdir),csu)
sysdep_routines += aeabi_read_tp libc-aeabi_read_tp
static-only-routines += aeabi_read_tp
shared-only-routines += libc-aeabi_read_tp
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += aeabi_read_tp
endif

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
