# The mpn functions need a #define for asm syntax flavor.
# Every i386 port in use uses gas syntax (I think).
asm-CPPFLAGS += -DGAS_SYNTAX

# The i386 `long double' is a distinct type we support.
long-double-fcts = yes

ifeq ($(subdir),csu)
# On i686 we must avoid generating the trampoline functions generated
# to get the GOT pointer.
CFLAGS-initfini.s += -march=i386 -mcpu=i386
endif

ifeq ($(subdir),db2)
CPPFLAGS += -DHAVE_SPINLOCKS=1 -DHAVE_ASSEM_X86_GCC=1
endif

ifeq ($(subdir),gmon)
sysdep_routines += i386-mcount
endif

ifeq ($(subdir),elf)
CFLAGS-rtld.c += -Wno-uninitialized -Wno-unused
CFLAGS-dl-load.c += -Wno-unused
CFLAGS-dl-reloc.c += -Wno-unused
endif
