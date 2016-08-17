ifeq ($(subdir),resource)
sysdep_routines += backtrace_linux
endif

ifeq ($(subdir),nptl)
# pull in __syscall_error routine
libpthread-routines += sysdep
libpthread-shared-only-routines += sysdep
endif
