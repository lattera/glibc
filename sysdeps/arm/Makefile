ifeq ($(subdir),elf)
sysdep-dl-routines += tlsdesc dl-tlsdesc
sysdep_routines += tlsdesc dl-tlsdesc
sysdep-rtld-routines += tlsdesc dl-tlsdesc
endif

ifeq ($(subdir),csu)
gen-as-const-headers += tlsdesc.sym
endif

# to pull in __aeabi_read_tp, needed for tls
ifeq ($(subdir),malloc)
$(objpfx)libmemusage.so: $(common-objpfx)libc_nonshared.a
endif
