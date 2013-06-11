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
# Add benchmark functions in alphabetical order.

subdir := benchtests
bench := acos acosh asin asinh atan atanh cos cosh exp log modf pow rint sin \
	 sinh tan tanh

# String function benchmarks.
string-bench := bcopy bzero memccpy memchr memcmp memmem memmove mempcpy \
		memset rawmemchr stpcpy stpncpy strcasecmp strcasestr strcat \
		strchr strchrnul strcmp strcpy strcspn strlen strncasecmp \
		strncat strncmp strncpy strnlen strpbrk strrchr strspn strstr \
		strcpy_chk stpcpy_chk
string-bench-ifunc := $(addsuffix -ifunc, $(string-bench))
string-bench-all := $(string-bench) $(string-bench-ifunc)

benchset := $(string-bench-all)

acos-ARGLIST = double
acos-RET = double
LDFLAGS-bench-acos = -lm

acosh-ARGLIST = double
acosh-RET = double
LDFLAGS-bench-acosh = -lm

asin-ARGLIST = double
asin-RET = double
LDFLAGS-bench-asin = -lm

asinh-ARGLIST = double
asinh-RET = double
LDFLAGS-bench-asinh = -lm

atan-ARGLIST = double
atan-RET = double
LDFLAGS-bench-atan = -lm

atanh-ARGLIST = double
atanh-RET = double
LDFLAGS-bench-atanh = -lm

cos-ARGLIST = double
cos-RET = double
LDFLAGS-bench-cos = -lm

cosh-ARGLIST = double
cosh-RET = double
LDFLAGS-bench-cosh = -lm

exp-ARGLIST = double
exp-RET = double
LDFLAGS-bench-exp = -lm

log-ARGLIST = double
log-RET = double
LDFLAGS-bench-log = -lm

pow-ARGLIST = double:double
pow-RET = double
LDFLAGS-bench-pow = -lm

rint-ARGLIST = double
rint-RET = double
LDFLAGS-bench-rint = -lm

sin-ARGLIST = double
sin-RET = double
LDFLAGS-bench-sin = -lm

sinh-ARGLIST = double
sinh-RET = double
LDFLAGS-bench-sinh = -lm

tan-ARGLIST = double
tan-RET = double
LDFLAGS-bench-tan = -lm

tanh-ARGLIST = double
tanh-RET = double
LDFLAGS-bench-tanh = -lm



# Rules to build and execute the benchmarks.  Do not put any benchmark
# parameters beyond this point.

# We don't want the benchmark programs to run in parallel since that could
# affect their performance.
.NOTPARALLEL:

include ../Makeconfig
include ../Rules

binaries-bench := $(addprefix $(objpfx)bench-,$(bench))
binaries-benchset := $(addprefix $(objpfx)bench-,$(benchset))

# The default duration: 10 seconds.
ifndef BENCH_DURATION
BENCH_DURATION := 10
endif

CPPFLAGS-nonlib += -DDURATION=$(BENCH_DURATION)

# Use clock_gettime to measure performance of functions.  The default is to use
# HP_TIMING if it is available.
ifdef USE_CLOCK_GETTIME
CPPFLAGS-nonlib += -DUSE_CLOCK_GETTIME
endif

# This makes sure CPPFLAGS-nonlib and CFLAGS-nonlib are passed
# for all these modules.
cpp-srcs-left := $(binaries-benchset:=.c) $(binaries-bench:=.c)
lib := nonlib
include $(patsubst %,$(..)cppflags-iterator.mk,$(cpp-srcs-left))

bench-deps := bench-skeleton.c bench-timing.h Makefile

run-bench = $(test-wrapper-env) \
	    GCONV_PATH=$(common-objpfx)iconvdata LC_ALL=C \
	    $($*-ENV) $(rtld-prefix) $${run}

bench-clean:
	rm -f $(binaries-bench) $(addsuffix .o,$(binaries-bench))
	rm -f $(binaries-benchset) $(addsuffix .o,$(binaries-benchset))

bench: bench-set bench-func

bench-set: $(binaries-benchset)
	for run in $^; do \
	  echo "Running $${run}"; \
	  $(run-bench) > $${run}.out; \
	done

bench-func: $(binaries-bench)
	{ for run in $^; do \
	  echo "Running $${run}" >&2; \
	  $(run-bench); \
	done; } > $(objpfx)bench.out-tmp; \
	if [ -f $(objpfx)bench.out ]; then \
	  mv -f $(objpfx)bench.out $(objpfx)bench.out.old; \
	fi; \
	mv -f $(objpfx)bench.out-tmp $(objpfx)bench.out

$(binaries-bench) $(binaries-benchset): %: %.o \
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
