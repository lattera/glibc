sysdep-CFLAGS += -mcpu=ultrasparc -Wa,-Av9a

ifeq ($(subdir),csu)
sysdep_routines += hp-timing
elide-routines.os += hp-timing
endif

ifeq ($(have-as-vis3),yes)
ASFLAGS-.o += -Wa,-Av9d
ASFLAGS-.os += -Wa,-Av9d
ASFLAGS-.op += -Wa,-Av9d
ASFLAGS-.og += -Wa,-Av9d
ASFLAGS-.ob += -Wa,-Av9d
ASFLAGS-.oS += -Wa,-Av9d
else
ASFLAGS-.o += -Wa,-Av9a
ASFLAGS-.os += -Wa,-Av9a
ASFLAGS-.op += -Wa,-Av9a
ASFLAGS-.og += -Wa,-Av9a
ASFLAGS-.ob += -Wa,-Av9a
ASFLAGS-.oS += -Wa,-Av9a
endif
