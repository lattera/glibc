# We don't support long doubles as a distinct type.  We don't need to set
# this variable; it's here mostly for documentational purposes.

long-double-fcts = no

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),elf)
# Extra shared linker files to link only into dl-allobjs.so.
sysdep-rtld-routines += dl-start __tls_get_addr
endif
