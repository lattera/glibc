sysdep-CFLAGS += -mcpu=v8 -mtune=ultrasparc -Wa,-Av9a

ifeq ($(subdir),csu)
sysdep_routines += hp-timing
endif

ifeq ($(subst gnulib,string,$(subdir)),string)
ASFLAGS-.o += -Wa,-Av9a
ASFLAGS-.os += -Wa,-Av9a
ASFLAGS-.op += -Wa,-Av9a
ASFLAGS-.og += -Wa,-Av9a
ASFLAGS-.ob += -Wa,-Av9a
ASFLAGS-.oS += -Wa,-Av9a
endif
