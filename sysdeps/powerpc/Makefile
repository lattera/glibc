ifeq ($(subdir),math)
libm-support += fenv_const fe_nomask t_sqrt
endif

ifeq ($(subdir),gmon)
sysdep_routines += ppc-mcount
endif

# On PPC, -fpic works until the GOT contains 2^15 bytes, and possibly
# more depending on how clever the linker is.  Each GOT entry takes 4 bytes,
# so that's at least 8192 entries.  Since libc only uses about 1200 entries,
# we want to use -fpic, because this generates fewer relocs.
ifeq (yes,$(build-shared))
pic-ccflag = -fpic
endif

# The initfini generation code doesn't work in the presence of -fPIC, so
# we use -fpic instead which is much better.
ifeq ($(subdir),csu)
CFLAGS-initfini.s = -g0 -fpic
endif

ifeq ($(subdir),string)
CFLAGS-memcmp.c += -Wno-uninitialized
endif
