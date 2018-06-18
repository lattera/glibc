# When building float128 we need to ensure -mfloat128 is
# passed to all such object files.

# libgcc requires __tcb_parse_hwcap_and_convert_at_platform when built with
# a binary128 type.  That symbol is provided by the loader on dynamically
# linked executables, forcing to link the loader after libgcc link.
f128-loader-link = $(as-needed) $(elf-objpfx)ld.so $(no-as-needed)

ifeq ($(subdir),math)
# sqrtf128 requires emulation before POWER9.
CPPFLAGS += -I../soft-fp

# float128 requires adding a handful of extra flags.
$(foreach suf,$(all-object-suffixes),%f128$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),%f128_r$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),$(objpfx)test-float128%$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),$(objpfx)test-float64x%$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),$(objpfx)test-ifloat128%$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),$(objpfx)test-ifloat64x%$(suf)): CFLAGS += -mfloat128
# Pairs of types with _Float128 / _Float64x as the wider type but not
# the narrower one.
f128-pairs = float32-float64x float32-float128 float64-float64x \
	     float64-float128 float32x-float64x float32x-float128
$(foreach suf,$(all-object-suffixes),$(foreach pair,$(f128-pairs),$(objpfx)test-$(pair)%$(suf))): CFLAGS += -mfloat128
CFLAGS-libm-test-support-float128.c += -mfloat128
CFLAGS-libm-test-support-float64x.c += -mfloat128
CFLAGS-test-math-iscanonical.cc += -mfloat128
CFLAGS-test-math-iseqsig.cc += -mfloat128
CFLAGS-test-math-issignaling.cc += -mfloat128
CFLAGS-test-math-iszero.cc += -mfloat128
$(foreach test, \
	  test-float128% test-ifloat128% test-float64x% test-ifloat64x% \
	  $(foreach pair,$(f128-pairs),test-$(pair)%) \
	  test-math-iscanonical test-math-iseqsig test-math-issignaling \
	  test-math-iszero, \
	  $(objpfx)$(test)): \
  gnulib-tests += $(f128-loader-link)
endif

# Append flags to string <-> _Float128 routines.
ifneq ($(filter $(subdir),wcsmbs stdlib),)
$(foreach suf,$(all-object-suffixes),%f128$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),%f128_l$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),%f128_nan$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),%float1282mpn$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),%mpn2float128$(suf)): CFLAGS += -mfloat128
CFLAGS-bug-strtod.c += -mfloat128
CFLAGS-bug-strtod2.c += -mfloat128
CFLAGS-tst-strtod-round.c += -mfloat128
CFLAGS-tst-wcstod-round.c += -mfloat128
CFLAGS-tst-strtod-nan-locale.c += -mfloat128
CFLAGS-tst-wcstod-nan-locale.c += -mfloat128
CFLAGS-tst-strtod6.c += -mfloat128
CFLAGS-tst-strfrom.c += -mfloat128
CFLAGS-tst-strfrom-locale.c += -mfloat128
CFLAGS-strfrom-skeleton.c += -mfloat128
CFLAGS-tst-strtod-nan-sign.c += -mfloat128
CFLAGS-tst-wcstod-nan-sign.c += -mfloat128
$(foreach test,bug-strtod bug-strtod2 bug-strtod2 tst-strtod-round \
tst-wcstod-round tst-strtod6 tst-strrom tst-strfrom-locale \
tst-strtod-nan-locale tst-wcstod-nan-locale \
strfrom-skeleton tst-strtod-nan-sign tst-wcstod-nan-sign, \
$(objpfx)$(test)): gnulib-tests += $(f128-loader-link)

# When building glibc with support for _Float128, the powers of ten tables in
# fpioconst.c and in the string conversion functions must be extended.  Some
# Makefiles (e.g.: wcsmbs/Makefile) override CFLAGS defined by the Makefiles in
# sysdeps.  This is avoided with the use sysdep-CFLAGS instead of CFLAGS.
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
