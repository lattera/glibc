/* Traps for Mach basic system calls and kernel RPC shortcuts.  */

#include <sysdep.h>

#define	SYSCALL(name, number, type, args, typed_args) \
  SYSCALL_TRAP (__##name, number)

/* Basic syscalls.  */
#include "mach_syscalls.h"

/* RPC shortcuts.  */
#define	SHORTCUT(name, number, args, typed_args) \
  SYSCALL_TRAP (__syscall_##name, number)

#include "mach_shortcuts.h"
