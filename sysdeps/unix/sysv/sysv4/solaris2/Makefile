# The linker supplied with Solaris looks in the current directory
# before searching others.  Compiling the various programs that come
# along the way (e.g., glue-ctype) will fail because it'll try to link
# with the libc.a being *constructed* in $(objdir).  As a work-around,
# we add this to each native-compile.
ALL_BUILD_CFLAGS += -L/lib
