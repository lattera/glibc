/* Macros to support TLS testing in times of missing compiler support.  */

#include <sys/cdefs.h>
#include <sys/asm.h>

#define __STRING2(X) __STRING(X)
#define ADDU __STRING2(PTR_ADDU)
#define ADDIU __STRING2(PTR_ADDIU)
#define LW __STRING2(PTR_L)

/* Load the GOT pointer, which may not be in $28 in a non-PIC
   (abicalls pic0) function.  */
#ifndef __PIC__
# if _MIPS_SIM != _ABI64
#  define LOAD_GP "move %[tmp], $28\n\tla $28, __gnu_local_gp\n\t"
# else
#  define LOAD_GP "move %[tmp], $28\n\tdla $28, __gnu_local_gp\n\t"
# endif
# define UNLOAD_GP "\n\tmove $28, %[tmp]"
#else
# define LOAD_GP
# define UNLOAD_GP
#endif

# define TLS_GD(x)					\
  ({ void *__result, *__tmp;				\
     extern void *__tls_get_addr (void *);		\
     asm (LOAD_GP ADDIU " %0, $28, %%tlsgd(" #x ")"	\
	  UNLOAD_GP					\
	  : "=r" (__result), [tmp] "=&r" (__tmp));	\
     (int *)__tls_get_addr (__result); })
# define TLS_LD(x)					\
  ({ void *__result, *__tmp;				\
     extern void *__tls_get_addr (void *);		\
     asm (LOAD_GP ADDIU " %0, $28, %%tlsldm(" #x ")"	\
	  UNLOAD_GP					\
	  : "=r" (__result), [tmp] "=&r" (__tmp));	\
     __result = __tls_get_addr (__result);		\
     asm ("lui $3,%%dtprel_hi(" #x ")\n\t"		\
	  "addiu $3,$3,%%dtprel_lo(" #x ")\n\t"		\
	  ADDU " %0,%0,$3"				\
	  : "+r" (__result) : : "$3");			\
     __result; })
# define TLS_IE(x)					\
  ({ void *__result, *__tmp;				\
     asm (".set push\n\t.set mips32r2\n\t"		\
	  "rdhwr\t%0,$29\n\t.set pop"			\
	  : "=v" (__result));				\
     asm (LOAD_GP LW " $3,%%gottprel(" #x ")($28)\n\t"	\
	  ADDU " %0,%0,$3"				\
	  UNLOAD_GP					\
	  : "+r" (__result), [tmp] "=&r" (__tmp)	\
	  : : "$3");					\
     __result; })
# define TLS_LE(x)					\
  ({ void *__result;					\
     asm (".set push\n\t.set mips32r2\n\t"		\
	  "rdhwr\t%0,$29\n\t.set pop"			\
	  : "=v" (__result));				\
     asm ("lui $3,%%tprel_hi(" #x ")\n\t"		\
	  "addiu $3,$3,%%tprel_lo(" #x ")\n\t"		\
	  ADDU " %0,%0,$3"				\
	  : "+r" (__result) : : "$3");			\
     __result; })
