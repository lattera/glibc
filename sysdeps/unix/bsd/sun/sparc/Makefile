# Basically `-e start' is magical to the Sun linker.  You would think that
# having start.o first would be enough, but you would be wrong.
LDFLAGS := $(LDFLAGS) -Xlinker -e -Xlinker start

ifeq ($(subdir),signal)
sysdep_routines := $(sysdep_routines) sigtramp
endif
