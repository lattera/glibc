#ifndef	_SYS_MODULE_H

#define	_SYS_MODULE_H	1
#include <features.h>

#define __need_size_t
#include <stddef.h>

#include <linux/module.h>

/* Return number of kernel symbols if TABLE == NULL, otherwise, return
   kernel symbols in TABLE.  TABLE must be large enough to hold all
   kernel symbols.  */
extern int get_kernel_syms __P ((struct kernel_sym * table));

/* Create a new module of name MODULE_NAME and of size SIZE bytes.
   The return address is the starting address of the new module or -1L
   if the module cannot be created (the return value needs to be cast
   to (long) to detect the error condition).  */
extern unsigned long create_module __P ((__const char * module_name,
					 size_t size));

/* Initialize the module called MODULE_NAME with the CONTENTSSIZE
   bytes starting at address CONTENTS.  CONTENTS normally contains the
   text and data segment of the module (the bss is implicity zeroed).
   After copying the contents, the function pointed to by
   ROUTINES.init is executed.  When the module is no longer needed,
   ROUTINES.cleanup is executed.  SYMTAB is NULL if the module does
   not want to export symbols by itself, or a pointer to a symbol
   table if the module wants to register its own symbols.  */
extern int init_module __P ((__const char * module_name,
			     __const void * contents, size_t contentssize,
			     struct mod_routines * routines,
			     struct symbol_table * symtab));

/* Delete the module named MODULE_NAME from the kernel.  */
extern int delete_module __P ((__const char *module_name));

#endif /* _SYS_MODULE_H */
