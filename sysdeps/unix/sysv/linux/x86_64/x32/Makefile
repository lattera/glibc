# The default ABI is x32.
default-abi := x32

ifeq ($(subdir),misc)
sysdep_routines += arch_prctl
endif

ifeq ($(subdir),posix)
sysdep_routines += getcpu sched_getcpu-static
endif
