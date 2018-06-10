# Used by *context() functions
ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

# Supporting non-executable stacks on HPPA requires changes to both
# the Linux kernel and glibc. The kernel currently needs an executable
# stack for syscall restarts and signal returns.
ifeq ($(subdir),elf)
test-xfail-check-execstack = yes
endif
