# We always want to use the new mnemonic syntax even if we are on a RS6000
# machine.
+cflags += -mnew-mnemonics -Wa,-mppc -mpowerpc
asm-CPPFLAGS += -Wa,-mppc

ifeq ($(subdir),math)
libm-support += fenv_const fe_nomask
endif

ifeq ($(subdir),gmon)
sysdep_routines += ppc-mcount
endif

ifeq ($(subdir),misc)
sysdep_routines += gprsave0 gprrest0 gprsave1 gprrest1 fprsave fprrest
endif

# On PPC, -fpic works until the GOT contains 2^15 bytes, and possibly
# more depending on how clever the linker is.  Each GOT entry takes 4 bytes,
# so that's at least 8192 entries.  Since libc only uses about 2000 entries,
# we want to use -fpic, because this generates fewer relocs.
ifeq (yes,$(build-shared))
pic-ccflag = -fpic
endif

ifeq ($(subdir),csu)
ifneq ($(elf),no)
# The initfini generation code doesn't work in the presence of -fPIC, so
# we use -fpic instead which is much better.
CFLAGS-initfini.s = -g0 -fpic -O1

# There is no benefit to using sdata for these objects, and the user
# of the library should be able to control what goes into sdata.
CFLAGS-init.o = -G0
CFLAGS-gmon-start.o = -G0
endif
endif

ifeq ($(subdir),string)
CFLAGS-memcmp.c += -Wno-uninitialized
endif

ifeq ($(subdir),elf)
# extra shared linker files to link into dl-allobjs.so and libc
sysdep-dl-routines += dl-machine
sysdep_routines += dl-machine
# extra shared linker files to link only into dl-allobjs.so
sysdep-rtld-routines += dl-machine dl-start
endif
