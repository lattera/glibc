ifeq ($(subdir),signal)
sysdep_routines += __sigtrampoline
endif

asm-CPPFLAGS=-D__ASSEMBLY__
as-FLAGS-.so=-fPIC

# When I get this to work, this is the right thing
ifeq ($(subdir),elf)
CFLAGS-rtld.c += -mv8
#rtld-routines += dl-sysdepsparc
endif   # elf
