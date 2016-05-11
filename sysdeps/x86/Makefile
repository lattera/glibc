ifeq ($(subdir),csu)
gen-as-const-headers += cpu-features-offsets.sym
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += dl-get-cpu-features

tests += tst-get-cpu-features
tests-static += tst-get-cpu-features-static
endif
