ifeq ($(subdir),math)
libm-support += fenv_const fe_nomask t_sqrt

# These routines have not been tested, so it's probable they don't work;
# and they don't provide the complete list of FP routines.  So there's
# no point in compiling them.
#sysdep_routines += q_feq q_fne q_utoq q_dtoq q_itoq q_stoq q_neg q_ulltoq \
#	           q_lltoq q_qtou q_qtoi q_qtoull q_qtoll q_qtos

tests += test-arith test-arithf
LDLIBS-test-arith = libm
LDLIBS-test-arithf = libm
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
