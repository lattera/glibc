/* copy sign, double version.
   Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <machine/asm.h>

ENTRY(__copysignf)
	movss  %xmm0,-4(%rsp)
	mov    -4(%rsp,1),%edx
	movss  %xmm1,-4(%rsp)
	mov    -4(%rsp),%eax
	and    $0x7fffffff,%edx
	and    $0x80000000,%eax
	or     %eax,%edx
	mov    %edx,-4(%rsp)
	movss  -4(%rsp),%xmm1
	movaps %xmm1,%xmm0
	retq
END (__copysignf)

weak_alias (__copysignf, copysignf)
