ifeq ($(subdir),string)
sysdep_routines += align-cpy
endif

ifeq ($(have-as-vis3),yes)
ASFLAGS-.o += -Wa,-Av9d
ASFLAGS-.os += -Wa,-Av9d
ASFLAGS-.op += -Wa,-Av9d
ASFLAGS-.og += -Wa,-Av9d
ASFLAGS-.oS += -Wa,-Av9d
endif

ifeq ($(subdir),nptl)
libpthread-routines += cpu_relax
endif
