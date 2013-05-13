# Copyright (C) 2013 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.


# Makefile for benchmark tests.  The only useful target here is `bench`.

# Adding a new function `foo`:
# ---------------------------

# - Append the function name to the bench variable

# - Define foo-ARGLIST as a colon separated list of types of the input
#   arguments.  Use `void` if function does not take any inputs.  Put in quotes
#   if the input argument is a pointer, e.g.:

#      malloc-ARGLIST: "void *"

# - Define foo-RET as the type the function returns.  Skip if the function
#   returns void.  One could even skip foo-ARGLIST if the function does not
#   take any inputs AND the function returns void.


# - Make a file called `foo-inputs` with one input value per line, an input
#   being a comma separated list of arguments to be passed into the function.
#   See pow-inputs for an example.

subdir := benchtests
bench := exp pow rint sin cos tan atan modf

exp-ARGLIST = double
exp-RET = double
LDFLAGS-bench-exp = -lm

pow-ARGLIST = double:double
pow-RET = double
LDFLAGS-bench-pow = -lm

rint-ARGLIST = double
rint-RET = double
LDFLAGS-bench-rint = -lm

sin-ARGLIST = double
sin-RET = double
LDFLAGS-bench-sin = -lm

cos-ARGLIST = double
cos-RET = double
LDFLAGS-bench-cos = -lm

tan-ARGLIST = double
tan-RET = double
LDFLAGS-bench-tan = -lm

atan-ARGLIST = double
atan-RET = double
LDFLAGS-bench-atan = -lm



# Rules to build and execute the benchmarks.  Do not put any benchmark
# parameters beyond this point.

include ../Makeconfig
include ../Rules

binaries-bench := $(addprefix $(objpfx)bench-,$(bench))

# The default duration: 10 seconds.
ifndef BENCH_DURATION
BENCH_DURATION := 10
endif

CPPFLAGS-nonlib = -DDURATION=$(BENCH_DURATION)

# Use clock_gettime to measure performance of functions.  The default is to use
# HP_TIMING if it is available.
ifdef USE_CLOCK_GETTIME
CPPFLAGS-nonlib += -DUSE_CLOCK_GETTIME
endif

# This makes sure CPPFLAGS-nonlib and CFLAGS-nonlib are passed
# for all these modules.
cpp-srcs-left := $(binaries-bench:=.c)
lib := nonlib
include $(patsubst %,$(..)cppflags-iterator.mk,$(cpp-srcs-left))

bench-deps := bench-skeleton.c bench-timing.h Makefile

run-bench = $(test-wrapper-env) \
	    GCONV_PATH=$(common-objpfx)iconvdata LC_ALL=C \
	    $($*-ENV) $(rtld-prefix) $${run}

bench-clean:
	rm -f $(binaries-bench) $(addsuffix .o,$(binaries-bench))

bench: $(binaries-bench)
	{ for run in $^; do \
	  echo "Running $${run}" >&2; \
	  $(run-bench); \
	done; } > $(objpfx)bench.out-tmp; \
	if [ -f $(objpfx)bench.out ]; then \
	  mv -f $(objpfx)bench.out $(objpfx)bench.out.old; \
	fi; \
	mv -f $(objpfx)bench.out-tmp $(objpfx)bench.out

$(binaries-bench): %: %.o \
  $(sort $(filter $(common-objpfx)lib%,$(link-libc))) \
  $(addprefix $(csu-objpfx),start.o) $(+preinit) $(+postinit)
	$(+link)

$(objpfx)bench-%.c: %-inputs $(bench-deps)
	{ if [ -n "$($*-INCLUDE)" ]; then \
	  cat $($*-INCLUDE); \
	fi; \
	$(..)scripts/bench.pl $(patsubst %-inputs,%,$<) \
	  $($*-ARGLIST) $($*-RET); } > $@-tmp
	mv -f $@-tmp $@
