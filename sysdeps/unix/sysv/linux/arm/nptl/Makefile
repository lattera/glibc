ifeq ($(subdir),rt)
librt-sysdep_routines += rt-aeabi_unwind_cpp_pr1
librt-shared-only-routines += rt-aeabi_unwind_cpp_pr1
endif

ifeq ($(subdir),nptl)
libpthread-sysdep_routines += nptl-aeabi_unwind_cpp_pr1
libpthread-shared-only-routines += nptl-aeabi_unwind_cpp_pr1

# This test relies on compiling part of the binary with EH information,
# part without, and unwinding through.  The .ARM.exidx tables have
# start addresses for EH regions, but no end addresses.  Every
# region an exception needs to propogate through must have unwind
# information, or a previous function's unwind table may be used
# by mistake.
tests := $(filter-out tst-cleanupx4,$(tests))
endif
