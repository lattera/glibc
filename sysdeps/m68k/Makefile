# Copyright (C) 1993 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

# This uses MIT assembler syntax.  We have no convenient
# way to choose a sysdep file based on MIT vs Motorola syntax.
# No existing m68k ports use Motorola syntax.

crypt := crypt.sun3	# Use crypt/crypt.sun3.S.

# Disgusting magic to get `#'s into the asm code.

# Set `as-pipe-ok' if piping input to the assembler is likely to work.
ifneq (,$(filter -pipe,$(compile.c) $(compile.S)))
# The `-pipe' switch the compiler does it, so it must work.
as-pipe-ok = yes
endif
ifdef gnu-as
# GNU as can deal with input pipes.
as-pipe-ok = yes
endif

ifdef as-pipe-ok

define compile-command.S
$(CC) $(CPPFLAGS) $(m68k-syntax-flag) $(asm-CPPFLAGS) -E $< \
| sed 's/(@@@Hash-Here@@@)/#/g' | $(AS) $(ASFLAGS) -o $@
endef

else

define compile-command.S
@-rm -f $@s
$(CC) $(CPPFLAGS) $(m68k-syntax-flag) $(asm-CPPFLAGS) -E $< \
| sed 's/(@@@Hash-Here@@@)/#/g' > $@s
$(AS) $(ASFLAGS) $@s -o $@
-rm -f $@s
endef

endif

# The mpn functions need this.  All existing 68k ports use MIT syntax.  If
# a new port wants to use Motorola or Sony syntax, it can redefine this
# variable.
ifndef m68k-syntax-flag
m68k-syntax-flag = -DMIT_SYNTAX
endif
