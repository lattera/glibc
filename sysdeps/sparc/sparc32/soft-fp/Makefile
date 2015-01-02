#  Software floating-point emulation.
#  Makefile for SPARC v8 long double utility functions (_Q_*).
#  Copyright (C) 1999-2015 Free Software Foundation, Inc.
#  This file is part of the GNU C Library.
#  Contributed by Jakub Jelinek (jj@ultra.linux.cz).
#

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

ifeq ($(subdir),soft-fp)
sparc32-quad-routines := q_add q_cmp q_cmpe q_div q_dtoq q_feq q_fge	\
	q_fgt q_fle q_flt q_fne q_itoq q_mul q_neg q_qtod q_qtoi	\
	q_qtos q_qtou q_qtoull q_qtoll q_sqrt q_stoq q_sub q_utoq	\
	q_ulltoq q_lltoq q_util
sysdep_routines += $(sparc32-quad-routines)

endif
