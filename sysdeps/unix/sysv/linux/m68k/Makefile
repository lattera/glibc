# Linux/m68k uses Motorola asm syntax and the ELF format.

m68k-syntax-flag = -DMOTOROLA_SYNTAX

ifeq ($(subdir),csu)
sysdep_routines += m68k-helpers
endif

ifeq ($(subdir),misc)
sysdep_routines += mremap
sysdep_headers += sys/reg.h
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += dl-static
sysdep_routines += dl-vdso libc-m68k-vdso dl-static
sysdep-rtld-routines += m68k-vdso dl-static
sysdep-others += lddlibc4
install-bin += lddlibc4
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif
