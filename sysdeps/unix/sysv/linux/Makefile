ifeq ($(subdir), io)
sysdep_routines := $(sysdep_routines) fxstat lxstat xstat
endif

ifeq ($(subdir), misc)
sysdep_routines := $(sysdep_routines) xmknod
endif

#ifeq ($(subdir), posix)
#sysdep_routines := $(sysdep_routines) setpgrp
#endif

ifeq ($(subdir), time)
sysdep_routines := $(sysdep_routines) adjtimex
endif

ifeq ($(subdir), sysvipc)
sysdep_routines := $(sysdep_routines) ipc
endif

headers := $(headers) sys/socketcall.h sys/timex.h
