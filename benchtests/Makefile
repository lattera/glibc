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

# - Define foo-ITER with the number of iterations you want to run.  Keep it
#   high enough that the overhead of clock_gettime is only a small fraction of
#   the total run time of the test.

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
bench := exp pow

exp-ITER = 100000
exp-ARGLIST = double
exp-RET = double
LDFLAGS-bench-exp = -lm

pow-ITER = 100000
pow-ARGLIST = double:double
pow-RET = double
LDFLAGS-bench-pow = -lm

include ../Makeconfig
include ../Rules
