# powerpc64/ELF specific definitions.

# Need to prevent gcc from using fprs in code used during dynamic linking.

CFLAGS-dl-runtime.os := -msoft-float
CFLAGS-dl-lookup.os := -msoft-float
CFLAGS-dl-misc.os := -msoft-float
CFLAGS-rtld-mempcpy.os := -msoft-float
CFLAGS-rtld-memmove.os := -msoft-float
CFLAGS-rtld-memchr.os := -msoft-float
CFLAGS-rtld-strnlen.os := -msoft-float
