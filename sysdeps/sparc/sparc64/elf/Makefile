# The assembler on SPARC needs the -fPIC flag even when it's assembler code.
ASFLAGS-.os = -fPIC

ifeq ($(subdir), csu)
extra-objs += crtbegin.o crtend.o crtbeginS.o crtendS.o
install-lib += crtbegin.o crtend.o crtbeginS.o crtendS.o

CPPFLAGS-crtbeginS.S = -fPIC -DPIC
CPPFLAGS-crtendS.S = -fPIC -DPIC
endif
