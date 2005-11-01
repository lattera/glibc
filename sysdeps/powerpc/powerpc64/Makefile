# Powerpc64 specific build options.
# this is ./sysdeps/powerpc/powerpc64/Makefile

# Each TOC entry takes 8 bytes and the TOC holds up to 2^16 bytes,
# or 8192 entries.
# If -fpic is not specified, the latest gcc-3.2.1 now generates
# different code for call stubs (without the TOC reload).
# Shared objects need the TOC reload so specify -fpic.
ifeq (yes,$(build-shared))
pic-ccflag = -fpic
endif

ifeq ($(subdir),csu)
sysdep_routines += hp-timing
elide-routines.os += hp-timing
ifneq ($(elf),no)
# The initfini generation code doesn't work in the presence of -fPIC, so
# we use -fpic instead which is much better.
CFLAGS-initfini.s += -fpic -O1
endif
endif

ifeq ($(subdir),elf)
# help gcc inline asm code from dl-machine.h
+cflags += -finline-limit=2000
endif

ifeq ($(subdir),gmon)
# The assembly functions assume that fp arg regs are not trashed.
# Compiling with -msoft-float ensures that fp regs are not used
# for moving memory around.
CFLAGS-mcount.c += -msoft-float
endif
