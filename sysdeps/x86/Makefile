ifeq ($(subdir),elf)
CFLAGS-.os += $(if $(filter $(@F),$(patsubst %,%.os,$(all-rtld-routines))),\
		   -mno-sse -mno-mmx)

tests-special += $(objpfx)tst-ld-sse-use.out
$(objpfx)tst-ld-sse-use.out: ../sysdeps/x86/tst-ld-sse-use.sh $(objpfx)ld.so
	@echo "Checking ld.so for SSE register use.  This will take a few seconds..."
	$(BASH) $< $(objpfx) '$(NM)' '$(OBJDUMP)' '$(READELF)' > $@; \
	$(evaluate-test)
endif

ifeq ($(subdir),csu)
gen-as-const-headers += cpu-features-offsets.sym rtld-global-offsets.sym
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += dl-get-cpu-features

tests += tst-get-cpu-features
tests-static += tst-get-cpu-features-static
endif
