# Copyright (C) 1991,1992,1994,1995,1997,2005,2008
# Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

subdir := resource

headers	  := sys/resource.h bits/resource.h sys/vlimit.h sys/vtimes.h	\
	     ulimit.h

routines := getrlimit setrlimit getrlimit64 setrlimit64 getrusage ulimit      \
	    vlimit vtimes getpriority setpriority nice

tests = tst-getrlimit bug-ulimit1

include ../Rules
