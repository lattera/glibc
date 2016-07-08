# Makefile for the Python pretty printers.
#
# Copyright (C) 2016 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

# This contains rules for building and running the pretty printer tests.

subdir := pretty-printers
tests-need-hardcoded-path := yes

include ../Makeconfig

PYTHON := python

tests-pretty-printers := test-mutex-attributes test-mutex-printer \
			 test-condvar-attributes test-condvar-printer \
			 test-rwlock-attributes test-rwlock-printer

# Add the test programs to test-srcs so that they'll be compiled regardless
# of whether we should actually run them.
test-srcs := $(tests-pretty-printers)

ifeq ($(build-shared),yes)
nptl-tests-libs := $(shared-thread-library)
else
nptl-tests-libs := $(static-thread-library)
endif

# The test programs need to be compiled without optimizations so they won't
# confuse gdb.  We could use either the 'GCC optimize' pragma or the 'optimize'
# function attribute to achieve this; however, at least on ARM, gcc always
# produces different debugging symbols when invoked with a -O greater than 0
# than when invoked with -O0, regardless of anything else we're using
# to suppress optimizations.  Therefore, we need to explicitly pass -O0 to it
# through CFLAGS.
# Additionally, the build system will try to -include $(common-objpfx)/config.h
# when compiling the tests, which will throw an error if some special macros
# (such as __OPTIMIZE__ and IS_IN_BUILD) aren't defined.  To avoid this, we
# tell gcc to define IS_IN_build.
CFLAGS-test-mutex-attributes.c := -O0 -ggdb3 -DIS_IN_build
CFLAGS-test-mutex-printer.c := -O0 -ggdb3 -DIS_IN_build
CFLAGS-test-condvar-attributes.c := -O0 -ggdb3 -DIS_IN_build
CFLAGS-test-condvar-printer.c := -O0 -ggdb3 -DIS_IN_build
CFLAGS-test-rwlock-attributes.c := -O0 -ggdb3 -DIS_IN_build
CFLAGS-test-rwlock-printer.c := -O0 -ggdb3 -DIS_IN_build

tests-pretty-printers-dest := $(addprefix $(objpfx),$(tests-pretty-printers))
tests-pretty-printers-pp := $(addsuffix -pp,$(tests-pretty-printers-dest))

ifeq ($(run-built-tests),yes)
tests-special += $(tests-pretty-printers-pp)
endif

include ../Rules

# Add the thread libraries to the prerequisites of the NPTL test programs.
$(tests-pretty-printers-dest): $(nptl-tests-libs)

# We won't actually create any *-pp files, so we mark this target as PHONY
# to ensure it always runs when required.
.PHONY: $(tests-pretty-printers-pp)

# Static pattern rule that matches the test-* targets to their .c and .py
# prerequisites.  It'll run the corresponding test script for each test program
# we compiled.  test_common.py must be present for all.
$(tests-pretty-printers-pp): $(objpfx)%-pp: $(objpfx)% %.py test_common.py
	$(test-wrapper-env) $(PYTHON) $*.py $*.c $(objpfx)$*; \
	$(evaluate-test)
