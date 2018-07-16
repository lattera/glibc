ifeq ($(subdir),csu)
gen-as-const-headers += cpu-features-offsets.sym
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += dl-get-cpu-features

tests += tst-get-cpu-features
tests-static += tst-get-cpu-features-static
endif

ifeq ($(subdir),setjmp)
gen-as-const-headers += jmp_buf-ssp.sym
sysdep_routines += __longjmp_cancel
endif

ifeq ($(enable-cet),yes)
ifeq ($(subdir),elf)
sysdep-dl-routines += dl-cet
endif

# Add -fcf-protection to CFLAGS when CET is enabled.
CFLAGS-.o += -fcf-protection
CFLAGS-.os += -fcf-protection
CFLAGS-.op += -fcf-protection
CFLAGS-.oS += -fcf-protection

# Compile assembly codes with <cet.h> when CET is enabled.
asm-CPPFLAGS += -fcf-protection -include cet.h

ifeq ($(subdir),elf)
ifeq (yes,$(build-shared))
tests-special += $(objpfx)check-cet.out
endif

# FIXME: Can't use all-built-dso in elf/Makefile since this file is
# processed before elf/Makefile.  Duplicate it here.
cet-built-dso := $(common-objpfx)elf/ld.so $(common-objpfx)libc.so \
		 $(filter-out $(common-objpfx)linkobj/libc.so, \
			      $(sort $(wildcard $(addprefix $(common-objpfx), \
							    */lib*.so \
							    iconvdata/*.so))))

$(cet-built-dso:=.note): %.note: %
	@rm -f $@T
	LC_ALL=C $(READELF) -n $< > $@T
	test -s $@T
	mv -f $@T $@
common-generated += $(cet-built-dso:$(common-objpfx)%=%.note)

$(objpfx)check-cet.out: $(..)sysdeps/x86/check-cet.awk \
			$(cet-built-dso:=.note)
	LC_ALL=C $(AWK) -f $^ > $@; \
	$(evaluate-test)
generated += check-cet.out
endif
endif
