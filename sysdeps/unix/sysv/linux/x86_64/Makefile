32bit-predefine = __i386__
64bit-predefine = __x86_64__

ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl
sysdep_headers += sys/perm.h sys/reg.h sys/debugreg.h sys/io.h
endif

ifeq ($(subdir),stdlib)
sysdep_routines += __start_context
endif
