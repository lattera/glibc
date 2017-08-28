ifeq ($(subdir),signal)
#sysdep_routines += sigsuspend
endif

ifeq ($(subdir),misc)
sysdep_routines += cachectl cacheflush sysmips _test_and_set

sysdep_headers += sys/cachectl.h sys/sysmips.h sys/tas.h
endif

abi-variants := o32_soft o32_hard o32_soft_2008 o32_hard_2008
abi-variants += n32_soft n32_hard n32_soft_2008 n32_hard_2008
abi-variants += n64_soft n64_hard n64_soft_2008 n64_hard_2008

ifeq (,$(filter $(default-abi),$(abi-variants)))
Unknown ABI, must be one of $(abi-variants)
endif

abi-includes := sgidefs.h

# _MIPS_SIM_ABI32 == 1, _MIPS_SIM_NABI32 == 2, _MIPS_SIM_ABI64 == 3
abi-o32_soft-condition := !defined(__mips_nan2008) \
			  && defined(__mips_soft_float) \
			  && (_MIPS_SIM == _MIPS_SIM_ABI32)
abi-o32_hard-condition := !defined(__mips_nan2008) \
			  && defined(__mips_hard_float) \
			  && (_MIPS_SIM == _MIPS_SIM_ABI32)
abi-o32_soft_2008-condition := defined(__mips_nan2008) \
			       && defined(__mips_soft_float) \
			       && (_MIPS_SIM == _MIPS_SIM_ABI32)
abi-o32_hard_2008-condition := defined(__mips_nan2008) \
			       && defined(__mips_hard_float) \
			       && (_MIPS_SIM == _MIPS_SIM_ABI32)
abi-n32_soft-condition := !defined(__mips_nan2008) \
			  && defined(__mips_soft_float) \
			  && (_MIPS_SIM == _MIPS_SIM_NABI32)
abi-n32_hard-condition := !defined(__mips_nan2008) \
			  && defined(__mips_hard_float) \
			  && (_MIPS_SIM == _MIPS_SIM_NABI32)
abi-n32_soft_2008-condition := defined(__mips_nan2008) \
			       && defined(__mips_soft_float) \
			       && (_MIPS_SIM == _MIPS_SIM_NABI32)
abi-n32_hard_2008-condition := defined(__mips_nan2008) \
			       && defined(__mips_hard_float) \
			       && (_MIPS_SIM == _MIPS_SIM_NABI32)
abi-n64_soft-condition := !defined(__mips_nan2008) \
			  && defined(__mips_soft_float) \
			  && (_MIPS_SIM == _MIPS_SIM_ABI64)
abi-n64_hard-condition := !defined(__mips_nan2008) \
			  && defined(__mips_hard_float) \
			  && (_MIPS_SIM == _MIPS_SIM_ABI64)
abi-n64_soft_2008-condition := defined(__mips_nan2008) \
			       && defined(__mips_soft_float) \
			       && (_MIPS_SIM == _MIPS_SIM_ABI64)
abi-n64_hard_2008-condition := defined(__mips_nan2008) \
			       && defined(__mips_hard_float) \
			       && (_MIPS_SIM == _MIPS_SIM_ABI64)

ifeq ($(subdir),elf)
ifeq ($(build-shared),yes)
# This is needed for DSO loading from static binaries.
sysdep-dl-routines += dl-static

sysdep_routines += dl-vdso
endif

# Supporting non-executable stacks on MIPS requires changes to both
# the Linux kernel and glibc.  See
# <https://sourceware.org/ml/libc-alpha/2016-01/msg00567.html> and
# <https://sourceware.org/ml/libc-alpha/2016-01/msg00719.html>.
test-xfail-check-execstack = yes
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif
