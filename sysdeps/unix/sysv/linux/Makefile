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
headers += sys/timex.h
endif

ifeq ($(subdir), sysvipc)
sysdep_routines := $(sysdep_routines) ipc
endif

ifeq ($(subdir), socket)
headers += sys/socketcall.h 
endif

config-LDFLAGS = -Wl,-dynamic-linker=/lib/ld-gnu.so.1
