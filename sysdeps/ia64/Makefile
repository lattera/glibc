ifeq ($(subdir), csu)
CPPFLAGS-start.S = -D__ASSEMBLY__
endif

ifeq ($(subdir),elf)
dl-routines += dl-symaddr dl-fptr
rtld-routines += dl-symaddr dl-fptr
endif
