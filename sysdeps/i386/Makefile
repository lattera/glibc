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

ifeq ($(subdir),gmon)
sysdep_routines += i386-mcount
endif

ifeq ($(subdir),elf)
CFLAGS-rtld.c += -Wno-uninitialized -Wno-unused
CFLAGS-dl-load.c += -Wno-unused
CFLAGS-dl-reloc.c += -Wno-unused
endif

ifeq ($(subdir),debug)
CFLAGS-backtrace.c += -fexceptions
endif

# Most of the glibc routines don't ever call user defined callbacks
# nor use any FPU or SSE* and as such don't need bigger %esp alignment
# than 4 bytes.
# Lots of routines in math will use FPU, so make math subdir an exception
# here.
ifeq ($(subdir),math)
sysdep-CFLAGS += -mpreferred-stack-boundary=4
else
ifeq ($(subdir),csu)
sysdep-CFLAGS += -mpreferred-stack-boundary=4
else
sysdep-CFLAGS += -mpreferred-stack-boundary=2
# Likewise, any function which calls user callbacks
uses-callbacks += -mpreferred-stack-boundary=4
# Likewise, any stack alignment tests
stack-align-test-flags += -malign-double -mpreferred-stack-boundary=4
endif
endif

# And a couple of other routines
ifeq ($(subdir),stdlib)
CFLAGS-exit.c += -mpreferred-stack-boundary=4
endif
ifeq ($(subdir),elf)
CFLAGS-dl-init.c += -mpreferred-stack-boundary=4
CFLAGS-dl-fini.c += -mpreferred-stack-boundary=4
CFLAGS-dl-open.c += -mpreferred-stack-boundary=4
CFLAGS-dl-close.c += -mpreferred-stack-boundary=4
CFLAGS-dl-error.c += -mpreferred-stack-boundary=4
endif
ifeq ($(subdir),dlfcn)
CFLAGS-dlopen.c += -mpreferred-stack-boundary=4
CFLAGS-dlopenold.c += -mpreferred-stack-boundary=4
CFLAGS-dlclose.c += -mpreferred-stack-boundary=4
CFLAGS-dlerror.c += -mpreferred-stack-boundary=4
endif

ifneq (,$(filter -mno-tls-direct-seg-refs,$(CFLAGS)))
defines += -DNO_TLS_DIRECT_SEG_REFS
endif
