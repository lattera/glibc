# Powerpc64 specific build options.
# this is ./sysdeps/powerpc/powerpc64/Makefile

+cflags += -Wa,-mppc64 -mpowerpc64
asm-CPPFLAGS += -Wa,-mppc64

# Each TOC entry takes 8 bytes and the TOC holds up to 2^16 bytes,
# or 8192 entries.
# If -fpic is not specified, the latest gcc-3.2.1 now generates
# different code for call stubs (without the TOC reload).
# Shared objects need the TOC reload so specify -fpic.
ifeq (yes,$(build-shared))
pic-ccflag = -fpic
endif

ifeq ($(subdir),csu)
ifneq ($(elf),no)
# The initfini generation code doesn't work in the presence of -g1 or
# higher, so we use -g0.
CFLAGS-initfini.s = -g0 -fpic -O1
endif
endif

ifeq ($(subdir),elf)
# help gcc inline asm code from dl-machine.h
+cflags += --param max-inline-insns=2000
endif
