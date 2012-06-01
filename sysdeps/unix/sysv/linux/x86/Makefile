# We don't need any header files.
abi-includes :=

abi-variants := 32 64 x32

abi-32-options := -D__i386__ -U__x86_64__
abi-32-condition := !defined __x86_64__
abi-32-ld-soname := ld-linux.so.2
abi-64-options := -U__i386__ -D__x86_64__ -U__ILP32__ -D__LP64__
abi-64-condition := defined __x86_64__ && defined __LP64__
abi-64-ld-soname := ld-linux-x86-64.so.2
abi-x32-options := -U__i386__ -D__x86_64__ -D__ILP32__ -U__LP64__
abi-x32-condition := defined __x86_64__ && defined __ILP32__
abi-x32-ld-soname := ld-linux-x32.so.2

ifeq ($(subdir),misc)
sysdep_headers += sys/elf.h sys/perm.h sys/reg.h sys/vm86.h sys/debugreg.h sys/io.h
endif
