ifeq ($(subdir),nptl)
ifneq ($(arm-using-eabi),yes)
# These tests rely on PTHREAD_KEYS_MAX.  The SJLJ exception machinery
# in libgcc registers one key, however, so only PTHREAD_KEYS_MAX-1
# keys are available.  This works fine for EABI targets.
tests := $(filter-out tst-key1 tst-key4,$(tests))
endif
endif
