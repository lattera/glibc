syscall-list-variants := 32bit 64bit
syscall-list-32bit-options := -D__i386__ -U__x86_64__
syscall-list-32bit-condition := __WORDSIZE == 32
syscall-list-64bit-options := -U__i386__ -D__x86_64__
syscall-list-64bit-condition := __WORDSIZE == 64

ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl
sysdep_headers += sys/perm.h sys/reg.h sys/debugreg.h sys/io.h
endif

ifeq ($(subdir),stdlib)
sysdep_routines += __start_context
endif

ifeq ($(subdir),csu)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
endif
