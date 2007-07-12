# powerpc64/ELF specific definitions.

# Need to prevent gcc from using fprs in code used during dynamic linking.

CFLAGS-dl-runtime.os = $(no-special-regs)
CFLAGS-dl-lookup.os = $(no-special-regs)
CFLAGS-dl-misc.os = $(no-special-regs)
CFLAGS-rtld-mempcpy.os = $(no-special-regs)
CFLAGS-rtld-memmove.os = $(no-special-regs)
CFLAGS-rtld-memchr.os = $(no-special-regs)
CFLAGS-rtld-strnlen.os = $(no-special-regs)
