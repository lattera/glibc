# Makefile fragment for PowerPC with no FPU.

ifeq ($(subdir),soft-fp)
sysdep_routines += $(gcc-single-routines) $(gcc-double-routines) \
		   sim-full
endif

ifeq ($(subdir),math)
libm-support += fenv_const
CPPFLAGS += -I../soft-fp/
# The follow CFLAGS are a work around for GCC Bugzilla Bug 29253
# "expand_abs wrong default code for floating point"
# As this is not a regression, a fix is not likely to go into
# gcc-4.1.1 and may be too late for gcc-4.2.  So we need these flags
# until the fix in a gcc release and glibc drops support for earlier
# versions of gcc.
CFLAGS-e_powl.c += -fno-builtin-fabsl
CFLAGS-s_ccoshl.c += -fno-builtin-fabsl
CFLAGS-s_csinhl.c += -fno-builtin-fabsl
CFLAGS-s_clogl.c += -fno-builtin-fabsl
CFLAGS-s_clog10l.c += -fno-builtin-fabsl
CFLAGS-s_csinl.c += -fno-builtin-fabsl
CFLAGS-s_csqrtl.c += -fno-builtin-fabsl
endif
