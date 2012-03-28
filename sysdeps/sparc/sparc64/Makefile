ifeq ($(subdir),csu)
sysdep_routines += hp-timing
elide-routines.os += hp-timing
endif

ifeq ($(subdir),string)
sysdep_routines += align-cpy
endif

ifeq ($(have-as-vis3),yes)
ASFLAGS-.o += -Wa,-Av9d
ASFLAGS-.os += -Wa,-Av9d
ASFLAGS-.op += -Wa,-Av9d
ASFLAGS-.og += -Wa,-Av9d
ASFLAGS-.ob += -Wa,-Av9d
ASFLAGS-.oS += -Wa,-Av9d
endif
