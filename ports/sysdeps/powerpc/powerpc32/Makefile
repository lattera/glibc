# Some Powerpc32 variants assume soft-fp is the default even though there is
# an fp variant so provide -mhard-float if --with-fp is explicitly passed.

ifeq ($(with-fp),yes)
+cflags += -mhard-float
ASFLAGS += -mhard-float
sysdep-LDFLAGS += -mhard-float
endif
