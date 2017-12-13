include $(common-objpfx)cflags-mcmodel-large.mk

# Check for gcc to support the command-line switch, and for
# binutils to support the hwN_plt() assembly operators and relocations.
$(common-objpfx)cflags-mcmodel-large.mk: $(common-objpfx)config.make
	mcmodel=no; \
	(echo 'int main() { return getuid(); }' | \
	 $(CC) -o /dev/null -xc - -mcmodel=large -fpic) && mcmodel=yes; \
	echo "cflags-mcmodel-large = $$mcmodel" > $@

ifeq (yes,$(cflags-mcmodel-large))

ifeq ($(subdir),csu)
# elf-init.c is in libc_nonshared.o (the end of the shared object) but
# must reach the _init symbol at the very start of the shared object.
CFLAGS-elf-init.c += -mcmodel=large

# __gmon_start__ is at the very start of the shared object when linked
# with profiling, but calls to libc.so via the PLT at the very end.
CFLAGS-gmon-start.c += -mcmodel=large
endif

else

# Don't try to compile assembly code with hwN_plt() directives if the
# toolchain doesn't support -mcmodel=large.
ifeq ($(subdir),csu)
CPPFLAGS-start.S += -DNO_PLT_PCREL
CPPFLAGS-crti.S += -DNO_PLT_PCREL
endif
ifeq ($(subdir),nptl)
CPPFLAGS-pt-crti.S += -DNO_PLT_PCREL
endif

endif

# We don't support long doubles as a distinct type.  We don't need to set
# this variable; it's here mostly for documentational purposes.

long-double-fcts = no

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),elf)
# Extra shared linker files to link only into dl-allobjs.so.
sysdep-rtld-routines += dl-start __tls_get_addr
endif
