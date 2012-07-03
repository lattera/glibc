gnulib-arch = $(elfobjdir)/libgcc-stubs.a
static-gnulib-arch = $(elfobjdir)/libgcc-stubs.a

ifeq ($(subdir),elf)
sysdep-dl-routines += tlsdesc dl-tlsdesc
sysdep_routines += aeabi_unwind_cpp_pr1 find_exidx tlsdesc dl-tlsdesc
sysdep-rtld-routines += aeabi_unwind_cpp_pr1 tlsdesc dl-tlsdesc
shared-only-routines += aeabi_unwind_cpp_pr1

$(objpfx)libgcc-stubs.a: $(objpfx)aeabi_unwind_cpp_pr1.os
	$(build-extra-lib)

lib-noranlib: $(objpfx)libgcc-stubs.a
endif

ifeq ($(subdir),csu)
# get offset to rtld_global._dl_hwcap
gen-as-const-headers += rtld-global-offsets.sym tlsdesc.sym
aeabi_constants = aeabi_lcsts aeabi_sighandlers aeabi_math
aeabi_routines = aeabi_assert aeabi_localeconv aeabi_errno_addr \
	aeabi_mb_cur_max aeabi_atexit aeabi_memclr aeabi_memcpy \
	aeabi_memmove aeabi_memset

sysdep_routines += $(aeabi_constants) $(aeabi_routines)
static-only-routines += $(aeabi_constants)
endif

# to pull in __aeabi_read_tp, needed for tls
ifeq ($(subdir),malloc)
$(objpfx)libmemusage.so: $(common-objpfx)libc_nonshared.a
endif

ifeq ($(subdir),gmon)
sysdep_routines += arm-mcount
endif

ifeq ($(subdir),debug)
CFLAGS-backtrace.c += -funwind-tables
endif

ifeq ($(subdir),math)
$(objpfx)libm.so: $(elfobjdir)/ld.so
endif
