# Makefile fragment for ARM/NaCl configurations.

# Copyright (C) 2015 Free Software Foundation, Inc.
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

# sysdeps/nacl/Makefile needs this set to the architecture suffix used in
# the NaCl SDK.
nacl-sdk-arch = arm

# We don't really support TLSDESC, even though the compiler thinks it does.
have-arm-tls-desc = no

ifeq ($(subdir),csu)
sysdep_routines += aeabi_read_tp
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += aeabi_read_tp
endif
