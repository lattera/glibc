# This file is included several times in a row, once
# for each element of $(lib)-routines and $(lib)-sysdeps_routines.

cpp-src := $(firstword $(cpp-srcs-left))
cpp-srcs-left := $(filter-out $(cpp-src),$(cpp-srcs-left))

CPPFLAGS-$(cpp-src) += -DNOT_IN_libc -DIS_IN_$(lib)
