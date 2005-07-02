asm-CPPFLAGS = -D__ASSEMBLY__
ASFLAGS-.os += -fPIC
LD += -melf32_sparc

# When I get this to work, this is the right thing
ifeq ($(subdir),elf)
CFLAGS-rtld.c += -mcpu=v8
#rtld-routines += dl-sysdepsparc
sysdep-others += lddlibc4
install-bin += lddlibc4
endif   # elf
