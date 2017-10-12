sysdep-CFLAGS += -mcpu=ultrasparc -Wa,-Av9a -mvis

ASFLAGS-.o += -Wa,-Av9d
ASFLAGS-.os += -Wa,-Av9d
ASFLAGS-.op += -Wa,-Av9d
ASFLAGS-.oS += -Wa,-Av9d

# nscd uses atomic_spin_nop which in turn requires cpu_relax
ifeq ($(subdir),nscd)
routines += cpu_relax
endif

ifeq ($(subdir), nptl)
libpthread-routines += cpu_relax
endif
