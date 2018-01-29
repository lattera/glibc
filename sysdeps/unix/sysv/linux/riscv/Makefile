ifeq ($(subdir),elf)
sysdep_routines	+= dl-vdso
ifeq ($(build-shared),yes)
# This is needed for DSO loading from static binaries.
sysdep-dl-routines += dl-static
endif
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/cachectl.h
sysdep_routines += flush-icache
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

abi-variants := lp64 lp64d

ifeq (,$(filter $(default-abi),$(abi-variants)))
$(error Unknown ABI $(default-abi), must be one of $(abi-variants))
endif

abi-lp64-condition    := defined __LP64__ && defined __riscv_float_abi_soft
abi-lp64d-condition   := defined __LP64__ && defined __riscv_float_abi_double
