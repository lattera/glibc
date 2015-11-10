ifeq ($(subdir),csu)
gen-as-const-headers += cpu-features-offsets.sym rtld-global-offsets.sym
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += dl-get-cpu-features

tests += tst-get-cpu-features
tests-static += tst-get-cpu-features-static

tests += tst-prelink
tst-prelink-ENV = LD_TRACE_PRELINKING=1

$(objpfx)tst-prelink-conflict.out: $(objpfx)tst-prelink.out
	grep stdout $< | grep conflict | $(AWK) '{ print $$10, $$11 }' > $@

$(objpfx)tst-prelink-cmp.out: $(..)sysdeps/x86/tst-prelink.exp \
			      $(objpfx)tst-prelink-conflict.out
	cmp $^ > $@; \
	$(evaluate-test)

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)tst-prelink-cmp.out
endif
endif
