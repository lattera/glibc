# Linux/m68k uses Motorola asm syntax and the ELF format.

m68k-syntax-flag = -DMOTOROLA_SYNTAX

ifeq ($(subdir),misc)
sysdep_routines += mremap setfsgid setfsuid setresgid setresuid
sysdep_headers += sys/reg.h
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4

ifeq (yes,$(build-shared))
# This is needed to support g++ v2 and v3.
sysdep_routines += framestate
shared-only-routines += framestate
endif
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif
