# Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

include ../Makeconfig
bench-math := acos acosh asin asinh atan atanh cos cosh exp exp2 ffs ffsll \
	      log log2 modf pow rint sin sincos sinh sqrt tan tanh

bench-pthread := pthread_once

bench := $(bench-math) $(bench-pthread)

# String function benchmarks.
string-bench := bcopy bzero memccpy memchr memcmp memcpy memmem memmove \
		mempcpy memset rawmemchr stpcpy stpncpy strcasecmp strcasestr \
		strcat strchr strchrnul strcmp strcpy strcspn strlen \
		strncasecmp strncat strncmp strncpy strnlen strpbrk strrchr \
		strspn strstr strcpy_chk stpcpy_chk memrchr strsep strtok
string-bench-all := $(string-bench)

stdlib-bench := strtod

benchset := $(string-bench-all) $(stdlib-bench)

CFLAGS-bench-ffs.c += -fno-builtin
CFLAGS-bench-ffsll.c += -fno-builtin

bench-malloc := malloc-thread

$(addprefix $(objpfx)bench-,$(bench-math)): $(libm)
$(addprefix $(objpfx)bench-,$(bench-pthread)): $(shared-thread-library)
$(objpfx)bench-malloc-thread: $(shared-thread-library)



# Rules to build and execute the benchmarks.  Do not put any benchmark
# parameters beyond this point.

# We don't want the benchmark programs to run in parallel since that could
# affect their performance.
.NOTPARALLEL:

include ../Rules

binaries-bench := $(addprefix $(objpfx)bench-,$(bench))
binaries-benchset := $(addprefix $(objpfx)bench-,$(benchset))
binaries-bench-malloc := $(addprefix $(objpfx)bench-,$(bench-malloc))

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

DETAILED_OPT :=

ifdef DETAILED
DETAILED_OPT := -d
endif

# This makes sure CPPFLAGS-nonlib and CFLAGS-nonlib are passed
# for all these modules.
cpp-srcs-left := $(binaries-benchset:=.c) $(binaries-bench:=.c) \
		 $(binaries-bench-malloc:=.c)
lib := nonlib
include $(patsubst %,$(..)cppflags-iterator.mk,$(cpp-srcs-left))

extra-objs += json-lib.o

bench-deps := bench-skeleton.c bench-timing.h Makefile

run-bench = $(test-wrapper-env) \
	    $(run-program-env) \
	    $($*-ENV) $(rtld-prefix) $${run}

timing-type := $(objpfx)bench-timing-type

bench-clean:
	rm -f $(binaries-bench) $(addsuffix .o,$(binaries-bench))
	rm -f $(binaries-benchset) $(addsuffix .o,$(binaries-benchset))
	rm -f $(binaries-bench-malloc) $(addsuffix .o,$(binaries-bench-malloc))
	rm -f $(timing-type) $(addsuffix .o,$(timing-type))

bench: $(timing-type) bench-set bench-func bench-malloc

bench-set: $(binaries-benchset)
	for run in $^; do \
	  echo "Running $${run}"; \
	  $(run-bench) > $${run}.out; \
	done

bench-malloc: $(binaries-bench-malloc)
	run=$(objpfx)bench-malloc-thread; \
	for thr in 1 8 16 32; do \
	  echo "Running $${run} $${thr}"; \
	  $(run-bench) $${thr} > $${run}-$${thr}.out; \
	done

# Build and execute the benchmark functions.  This target generates JSON
# formatted bench.out.  Each of the programs produce independent JSON output,
# so one could even execute them individually and process it using any JSON
# capable language or tool.
bench-func: $(binaries-bench)
	{ timing_type=$$($(timing-type)); \
	echo "{\"timing_type\": \"$${timing_type}\","; \
	echo " \"functions\": {"; \
	for run in $^; do \
	  if ! [ "x$${run}" = "x$<" ]; then \
	    echo ","; \
	  fi; \
	  echo "Running $${run}" >&2; \
	  $(run-bench) $(DETAILED_OPT); \
	done; \
	echo; \
	echo " }"; \
	echo "}"; } > $(objpfx)bench.out-tmp; \
	if [ -f $(objpfx)bench.out ]; then \
	  mv -f $(objpfx)bench.out $(objpfx)bench.out.old; \
	fi; \
	mv -f $(objpfx)bench.out-tmp $(objpfx)bench.out
	scripts/validate_benchout.py $(objpfx)bench.out \
		scripts/benchout.schema.json

$(timing-type) $(binaries-bench) $(binaries-benchset) \
	$(binaries-bench-malloc): %: %.o $(objpfx)json-lib.o \
  $(sort $(filter $(common-objpfx)lib%,$(link-libc))) \
  $(addprefix $(csu-objpfx),start.o) $(+preinit) $(+postinit)
	$(+link)

$(objpfx)bench-%.c: %-inputs $(bench-deps)
	{ if [ -n "$($*-INCLUDE)" ]; then \
	  cat $($*-INCLUDE); \
	fi; \
	scripts/bench.py $(patsubst %-inputs,%,$<); } > $@-tmp
	mv -f $@-tmp $@
