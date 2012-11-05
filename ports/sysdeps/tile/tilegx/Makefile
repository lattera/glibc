include $(common-objpfx)cflags-mcmodel-large.mk

$(common-objpfx)cflags-mcmodel-large.mk: $(common-objpfx)config.make
	mcmodel=no; \
	$(CC) -S -o /dev/null -xc /dev/null -mcmodel=large && mcmodel=yes; \
	echo "cflags-mcmodel-large = $$mcmodel" > $@

ifeq ($(subdir),csu)
ifeq (yes,$(cflags-mcmodel-large))
# elf-init.c is in libc_nonshared.o (the end of the shared object) but
# must reach the _init symbol at the very start of the shared object.
CFLAGS-elf-init.c += -mcmodel=large

# __gmon_start__ is at the very start of the shared object when linked
# with profiling, but calls to libc.so via the PLT at the very end.
CFLAGS-gmon-start.c += -mcmodel=large
endif
endif
