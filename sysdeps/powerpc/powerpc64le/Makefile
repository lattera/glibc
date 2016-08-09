# When building float128 we need to ensure -mfloat128 is
# passed to all such object files.

ifeq ($(subdir),math)
# sqrtf128 requires emulation before POWER9.
CPPFLAGS += -I../soft-fp

# float128 requires adding a handful of extra flags.
$(foreach suf,$(all-object-suffixes),%f128$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),%f128_r$(suf)): CFLAGS += -mfloat128
$(objpfx)test-float128%.o $(objpfx)test-float128%.os: CFLAGS += -mfloat128
$(objpfx)test-ifloat128%.o $(objpfx)test-ifloat128%.os: CFLAGS += -mfloat128
CFLAGS-libm-test-support-float128.c += -mfloat128
endif

# Append flags to string <-> _Float128 routines.
ifneq ($(filter $(subdir),wcsmbs stdlib),)
%f128.o %f128.os %f128_l.o %f128_l.os %f128_nan.o %f128_nan.os %float1282mpn.o %float1282mpn.os %mpn2float128.o %mpn2float128.os: CFLAGS += -mfloat128
CFLAGS-bug-strtod.c += -mfloat128
CFLAGS-bug-strtod2.c += -mfloat128
CFLAGS-tst-strtod-round.c += -mfloat128
CFLAGS-tst-wcstod-round.c += -mfloat128
CFLAGS-tst-strtod6.c += -mfloat128
CFLAGS-tst-strfrom.c += -mfloat128
CFLAGS-tst-strfrom-locale.c += -mfloat128
CFLAGS-strfrom-skeleton.c += -mfloat128

# When building glibc with support for _Float128, the powers of ten tables in
# fpioconst.c and in the string conversion functions must be extended.
sysdep-CFLAGS += $(sysdep-CFLAGS-$(<F))
sysdep-CFLAGS-fpioconst.c += -mfloat128
sysdep-CFLAGS-strtod_l.c += -mfloat128
sysdep-CFLAGS-strtof_l.c += -mfloat128
sysdep-CFLAGS-strtold_l.c += -mfloat128
sysdep-CFLAGS-wcstod_l.c += -mfloat128
sysdep-CFLAGS-wcstof_l.c += -mfloat128
sysdep-CFLAGS-wcstold_l.c += -mfloat128
endif

# Append flags to printf routines.
ifeq ($(subdir),stdio-common)
CFLAGS-printf_fp.c = -mfloat128
CFLAGS-printf_fphex.c = -mfloat128
CFLAGS-printf_size.c = -mfloat128
endif
