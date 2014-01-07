ifeq ($(subdir),setjmp)
ifeq (yes,$(build-shared))
sysdep_routines += v1-longjmp v1-sigjmp
endif
endif

ifeq ($(subdir),csu)
gen-as-const-headers += rtld-global-offsets.sym
endif
