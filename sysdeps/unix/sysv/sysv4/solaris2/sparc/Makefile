# This flag is necessary because GCC now tries to call _Q_{mul, etc...}
# instead of doing the stuff the hard way.  For now, printf_fp.o, __vfscanf.o,
# and difftime.o don't work because of this.  The long-term fix is to actually
# implement what they're doing, but for the short-term, we must do this.
sysdep-CFLAGS := $(sysdep-CFLAGS) -mhard-quad-float

ifeq ($(subdir),crypt)

crypt := crypt.solar

endif
