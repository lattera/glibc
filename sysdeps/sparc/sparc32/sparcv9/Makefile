sysdep-CFLAGS += -mcpu=ultrasparc -Wa,-Av9a

ifeq ($(subdir),csu)
sysdep_routines += hp-timing
static-only-routines += hp-timing
endif

ASFLAGS-.o += -Wa,-Av9a
ASFLAGS-.os += -Wa,-Av9a
ASFLAGS-.op += -Wa,-Av9a
ASFLAGS-.og += -Wa,-Av9a
ASFLAGS-.ob += -Wa,-Av9a
ASFLAGS-.oS += -Wa,-Av9a
