# We need special startup code for statically linked binaries.
static-start-installed-name = crt0.o

ifeq ($(subdir),csu)
extra-objs += crt0.o
install-lib += crt0.o
omit-deps += crt0

$(objpfx)crt0.o: $(objpfx)static-start.o $(objpfx)abi-note.o $(objpfx)init.o
	$(link-relocatable)

# compat for libc.so.0.2 only
# This is needed to be backward-compatible with glibc-2.0.x startup code
# that was miscompiled so that it jumps to 0 if there is a nonzero
# __gmon_start__ symbol, but works if there is none.
CFLAGS-initfini.s += -DWEAK_GMON_START

endif
