/* Copyright (C) 1996 Free Software Foundation, Inc.
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

#ifndef	_SYS_MODULE_H

#define	_SYS_MODULE_H	1
#include <features.h>

#define __need_size_t
#include <stddef.h>

#include <linux/module.h>

__BEGIN_DECLS

/* Return number of kernel symbols if TABLE == NULL, otherwise, return
   kernel symbols in TABLE.  TABLE must be large enough to hold all
   kernel symbols.  */
extern int get_kernel_syms __P ((struct kernel_sym *__table));

/* Create a new module of name MODULE_NAME and of size SIZE bytes.
   The return address is the starting address of the new module or -1L
   if the module cannot be created (the return value needs to be cast
   to (long) to detect the error condition).  */
extern unsigned long int create_module __P ((__const char *__module_name,
					     size_t __size));

/* Initialize the module called MODULE_NAME with the CONTENTSSIZE
   bytes starting at address CONTENTS.  CONTENTS normally contains the
   text and data segment of the module (the bss is implicitly zeroed).
   After copying the contents, the function pointed to by
   ROUTINES.init is executed.  When the module is no longer needed,
   ROUTINES.cleanup is executed.  SYMTAB is NULL if the module does
   not want to export symbols by itself, or a pointer to a symbol
   table if the module wants to register its own symbols.  */
extern int init_module __P ((__const char *__module_name,
			     __const void *__contents, size_t __contentssize,
			     struct mod_routines *__routines,
			     struct symbol_table *__symtab));

/* Delete the module named MODULE_NAME from the kernel.  */
extern int delete_module __P ((__const char *__module_name));

__END_DECLS

#endif /* sys/module.h */
