# Powerpc64 specific build options.
# this is ./sysdeps/powerpc/powerpc64/Makefile

+cflags += -Wa,-mppc64 -mpowerpc64
asm-CPPFLAGS += -Wa,-mppc64

# On PPC64, -fpic is the default so we don't need to specify it. 
# Also early compilers would issue a warning if -fpic was specified.
# Each TOC entry takes 8 bytes and the TOC holds up to 2^16 bytes, 
# or 8192 entries.  If the TOC fills up try -minimal-toc.

ifeq ($(subdir),csu)
ifneq ($(elf),no)
# The initfini generation code doesn't work in the presence of -g1 or 
# higher, so we use -g0.
CFLAGS-initfini.s = -g0 -O1
endif
endif

ifeq ($(subdir),elf)
# help gcc inline asm code from dl-machine.h
+cflags += --param max-inline-insns=2000
endif
