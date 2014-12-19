ifeq ($(subdir),math)
# Since x32 returns 32-bit long int and 64-bit long long int in the
# same 64-bit register, we make the 32b-bit lround an alias of the
# 64-bit llround.  Add -fno-builtin-lround to silence the compiler.
CFLAGS-s_llround.c += -fno-builtin-lround
endif
