# The i387 `long double' is a distinct type we support.
long-double-fcts = yes

ifeq ($(subdir),csu)
sysdep_routines += hp-timing
elide-routines.os += hp-timing
gen-as-const-headers += link-defines.sym
endif

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),string)
sysdep_routines += cacheinfo strcasecmp_l-nonascii strncase_l-nonascii
gen-as-const-headers += locale-defines.sym
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += tlsdesc dl-tlsdesc
sysdep_routines += tlsdesc dl-tlsdesc
sysdep-rtld-routines += tlsdesc dl-tlsdesc

tests: $(objpfx)tst-xmmymm.out
$(objpfx)tst-xmmymm.out: ../sysdeps/x86_64/tst-xmmymm.sh $(objpfx)ld.so
	@echo "Checking ld.so for SSE register use.  This will take a few seconds..."
	$(SHELL) -e $< $(objpfx) > $@
endif

ifeq ($(subdir),csu)
gen-as-const-headers += tlsdesc.sym
endif
