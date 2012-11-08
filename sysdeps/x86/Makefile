ifeq ($(subdir),elf)
CFLAGS-.os += $(if $(filter $(@F),$(patsubst %,%.os,$(all-rtld-routines))),\
		   -mno-sse -mno-mmx)

tests: $(objpfx)tst-xmmymm.out
$(objpfx)tst-xmmymm.out: ../sysdeps/x86/tst-xmmymm.sh $(objpfx)ld.so
	@echo "Checking ld.so for SSE register use.  This will take a few seconds..."
	$(SHELL) $< $(objpfx) '$(NM)' '$(OBJDUMP)' '$(READELF)' > $@
endif
