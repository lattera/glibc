# Script to generate constants for Python pretty printers.
#
# Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

# This script is a smaller version of the clever gen-asm-const.awk hack used to
# generate ASM constants from .sym files.  We'll use this to generate constants
# for Python pretty printers.
#
# The input to this script are .pysym files that look like:
# #C_Preprocessor_Directive...
# NAME1
# NAME2 expression...
#
# A line giving just a name implies an expression consisting of just that name.
# Comments start with '--'.
#
# The output of this script is a 'dummy' function containing 'asm' declarations
# for each non-preprocessor line in the .pysym file.  The expression values
# will appear as input operands to the 'asm' declaration.  For example, if we
# have:
#
# /* header.h */
# #define MACRO 42
#
# struct S {
#     char c1;
#     char c2;
#     char c3;
# };
#
# enum E {
#     ZERO,
#     ONE
# };
#
# /* symbols.pysym */
# #include <stddef.h>
# #include "header.h"
# -- This is a comment
# MACRO
# C3_OFFSET offsetof(struct S, c3)
# E_ONE ONE
#
# the output will be:
#
# #include <stddef.h>
# #include "header.h"
# void dummy(void)
# {
#   asm ("@name@MACRO@value@%0@" : : "i" (MACRO));
#   asm ("@name@C3_OFFSET@value@%0@" : : "i" (offsetof(struct S, c3)));
#   asm ("@name@E_ONE@value@%0@" : : "i" (ONE));
# }
#
# We'll later feed this output to gcc -S.  Since '-S' tells gcc to compile but
# not assemble, gcc will output something like:
#
# dummy:
# 	...
# 	@name@MACRO@value@$42@
# 	@name@C3_OFFSET@value@$2@
# 	@name@E_ONE@value@$1@
#
# Finally, we can process that output to extract the constant values.
# Notice gcc may prepend a special character such as '$' to each value.

# found_symbol indicates whether we found a non-comment, non-preprocessor line.
BEGIN { found_symbol = 0 }

# C preprocessor directives go straight through.
/^#/ { print; next; }

# Skip comments.
/--/ { next; }

# Trim leading whitespace.
{ sub(/^[[:blank:]]*/, ""); }

# If we found a non-comment, non-preprocessor line, print the 'dummy' function
# header.
NF > 0 && !found_symbol {
    print "void dummy(void)\n{";
    found_symbol = 1;
}

# If the line contains just a name, duplicate it so we can use that name
# as the value of the expression.
NF == 1 { sub(/^.*$/, "& &"); }

# If a line contains a name and an expression...
NF > 1 {
    name = $1;

    # Remove any characters before the second field.
    sub(/^[^[:blank:]]+[[:blank:]]+/, "");

    # '$0' ends up being everything that appeared after the first field
    # separator.
    printf "  asm (\"@name@%s@value@%0@\" : : \"i\" (%s));\n", name, $0;
}

# Close the 'dummy' function.
END { if (found_symbol) print "}"; }
