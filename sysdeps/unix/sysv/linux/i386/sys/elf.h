/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _SYS_ELF_H
#define _SYS_ELF_H	1

/*
 * ELF register definitions..
 */

#include <sys/user.h>

typedef unsigned long elf_greg_t;

#define ELF_NGREG (sizeof (struct user_regs_struct) / sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef struct user_fpregs_struct elf_fpregset_t;

#define USE_ELF_CORE_DUMP
#define ELF_EXEC_PAGESIZE	4096

/* regs is struct pt_regs, pr_reg is elf_gregset_t (which is
   now struct_user_regs, they are different) */

#define ELF_CORE_COPY_REGS(pr_reg, regs)		\
	pr_reg[0] = regs->ebx;				\
	pr_reg[1] = regs->ecx;				\
	pr_reg[2] = regs->edx;				\
	pr_reg[3] = regs->esi;				\
	pr_reg[4] = regs->edi;				\
	pr_reg[5] = regs->ebp;				\
	pr_reg[6] = regs->eax;				\
	pr_reg[7] = regs->xds;				\
	pr_reg[8] = regs->xes;				\
	pr_reg[9] = regs->xds;				\
	pr_reg[10] = regs->xds;				\
	pr_reg[11] = regs->orig_eax;			\
	pr_reg[12] = regs->eip;				\
	pr_reg[13] = regs->xcs;				\
	pr_reg[14] = regs->eflags;			\
	pr_reg[15] = regs->esp;				\
	pr_reg[16] = regs->xss;

#endif	/* _SYS_ELF_H */
