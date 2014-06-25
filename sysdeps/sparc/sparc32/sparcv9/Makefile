sysdep-CFLAGS += -mcpu=ultrasparc -Wa,-Av9a

ifeq ($(have-as-vis3),yes)
ASFLAGS-.o += -Wa,-Av9d
ASFLAGS-.os += -Wa,-Av9d
ASFLAGS-.op += -Wa,-Av9d
ASFLAGS-.og += -Wa,-Av9d
ASFLAGS-.oS += -Wa,-Av9d
else
ASFLAGS-.o += -Wa,-Av9a
ASFLAGS-.os += -Wa,-Av9a
ASFLAGS-.op += -Wa,-Av9a
ASFLAGS-.og += -Wa,-Av9a
ASFLAGS-.oS += -Wa,-Av9a
endif

ifeq ($(subdir), nptl)
libpthread-routines += cpu_relax
endif
