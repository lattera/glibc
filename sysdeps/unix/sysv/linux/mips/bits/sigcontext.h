/* Copyright (C) 1996, 1997, 1998, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#if !defined _SIGNAL_H && !defined _SYS_UCONTEXT_H
# error "Never use <bits/sigcontext.h> directly; include <signal.h> instead."
#endif

#include <sgidefs.h>

#ifndef sigcontext_struct
/* Kernel headers before 2.1.1 define a struct sigcontext_struct, but
   we need sigcontext.  */
# define sigcontext_struct sigcontext

/* # include <asm/sigcontext.h> */
/* Instead of including the kernel header, that will vary depending on
   whether the 32- or the 64-bit kernel is installed, we paste the
   contents here.  In case you're wondering about the different
   licenses, the fact that the file is pasted, instead of included,
   doesn't really make any difference for the program that includes
   this header.  */
#if _MIPS_SIM == _ABIO32
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 1997, 2000 by Ralf Baechle
 */
#ifndef _ASM_SIGCONTEXT_H
#define _ASM_SIGCONTEXT_H

/*
 * Keep this struct definition in sync with the sigcontext fragment
 * in arch/mips/tools/offset.c
 */
struct sigcontext {
	unsigned int       sc_regmask;		/* Unused */
	unsigned int       sc_status;
	unsigned long long sc_pc;
	unsigned long long sc_regs[32];
	unsigned long long sc_fpregs[32];
	unsigned int       sc_ownedfp;		/* Unused */
	unsigned int       sc_fpc_csr;
	unsigned int       sc_fpc_eir;		/* Unused */
	unsigned int       sc_used_math;
	unsigned int       sc_ssflags;		/* Unused */
	unsigned long long sc_mdhi;
	unsigned long long sc_mdlo;

	unsigned int       sc_cause;		/* Unused */
	unsigned int       sc_badvaddr;		/* Unused */

	unsigned long      sc_sigset[4];	/* kernel's sigset_t */
};

#endif /* _ASM_SIGCONTEXT_H */
#else /* _MIPS_SIM != _ABIO32 */
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 1997, 1999 by Ralf Baechle
 * Copyright (C) 1999 Silicon Graphics, Inc.
 */
#ifndef _ASM_SIGCONTEXT_H
#define _ASM_SIGCONTEXT_H

/*
 * Keep this struct definition in sync with the sigcontext fragment
 * in arch/mips/tools/offset.c
 */
struct sigcontext {
	unsigned long long sc_regs[32];
	unsigned long long sc_fpregs[32];
	unsigned long long sc_mdhi;
	unsigned long long sc_mdlo;
	unsigned long long sc_pc;
	unsigned int       sc_status;
	unsigned int       sc_fpc_csr;
	unsigned int       sc_fpc_eir;
	unsigned int       sc_used_math;
	unsigned int       sc_cause;
	unsigned int       sc_badvaddr;
};

#endif /* _ASM_SIGCONTEXT_H */
#endif /* _MIPS_SIM != _ABIO32 */
#endif
