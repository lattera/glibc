asm-CPPFLAGS=-D__ASSEMBLY__
ASFLAGS-.os=-fPIC

ifeq ($(subdir),misc)
sysdep_routines += setfsgid setfsuid setresgid setresuid
endif

# When I get this to work, this is the right thing
ifeq ($(subdir),elf)
CFLAGS-rtld.c += -mv8
#rtld-routines += dl-sysdepsparc
endif   # elf
