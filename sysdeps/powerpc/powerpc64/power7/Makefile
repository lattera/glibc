ifeq ($(subdir),elf)
# Prevent the use of VSX registers and insns in _dl_start, which under -O3
# optimization may require a TOC reference before relocations are resolved.
CFLAGS-rtld.c += -mno-vsx
endif
