# The linker supplied with Solaris looks in the current directory
# before searching others.  Compiling the various programs that come
# along the way (e.g., glue-ctype) will fail because it'll try to link
# with the libc.a being *constructed* in $(objdir).  As a work-around,
# we add this to each native-compile.
native-CFLAGS := $(native-CFLAGS) -L/lib
# This flag is necessary because GCC now tries to call _Q_{mul, etc...}
# instead of doing the stuff the hard way.  For now, printf_fp.o, __vfscanf.o,
# and difftime.o don't work because of this.  The long-term fix is to actually
# implement what they're doing, but for the short-term, we must do this.
sysdep-CFLAGS := $(sysdep-CFLAGS) -mhard-quad-float
