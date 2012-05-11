/* Macros to support TLS testing in times of missing compiler support.  */

#define COMMON_INT_DEF(x) \
  asm (".tls_common " #x ",4,4")
/* XXX Until we get compiler support we don't need declarations.  */
#define COMMON_INT_DECL(x)

/* XXX This definition will probably be machine specific, too.  */
#define VAR_INT_DEF(x) \
  asm (".section .tdata\n\t"						      \
       ".globl " #x "\n"						      \
       ".balign 4\n"							      \
       #x ":\t.long 0\n\t"						      \
       ".size " #x ",4\n\t"						      \
       ".previous")
/* XXX Until we get compiler support we don't need declarations.  */
#define VAR_INT_DECL(x)

#include_next <tls-macros.h>

  /* XXX Each architecture must have its own asm for now.  */
#ifdef __i386__
# define TLS_LE(x) \
  ({ int *__l;								      \
     asm ("movl %%gs:0,%0\n\t"						      \
	  "subl $" #x "@tpoff,%0"					      \
	  : "=r" (__l));						      \
     __l; })

# ifdef PIC
#  define TLS_IE(x) \
  ({ int *__l;								      \
     asm ("movl %%gs:0,%0\n\t"						      \
	  "subl " #x "@gottpoff(%%ebx),%0"				      \
	  : "=r" (__l));						      \
     __l; })
# else
#  define TLS_IE(x) \
  ({ int *__l, __b;							      \
     asm ("call 1f\n\t"							      \
	  ".subsection 1\n"						      \
	  "1:\tmovl (%%esp), %%ebx\n\t"					      \
	  "ret\n\t"							      \
	  ".previous\n\t"						      \
	  "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"			      \
	  "movl %%gs:0,%0\n\t"						      \
	  "subl " #x "@gottpoff(%%ebx),%0"				      \
	  : "=r" (__l), "=&b" (__b));					      \
     __l; })
# endif

# ifdef PIC
#  define TLS_LD(x) \
  ({ int *__l, __c, __d;						      \
     asm ("leal " #x "@tlsldm(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "leal " #x "@dtpoff(%%eax), %%eax"				      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d));			      \
     __l; })
# else
#  define TLS_LD(x) \
  ({ int *__l, __b, __c, __d;						      \
     asm ("call 1f\n\t"							      \
	  ".subsection 1\n"						      \
	  "1:\tmovl (%%esp), %%ebx\n\t"					      \
	  "ret\n\t"							      \
	  ".previous\n\t"						      \
	  "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"			      \
	  "leal " #x "@tlsldm(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "leal " #x "@dtpoff(%%eax), %%eax"				      \
	  : "=a" (__l), "=&b" (__b), "=&c" (__c), "=&d" (__d));		      \
     __l; })
# endif

# ifdef PIC
#  define TLS_GD(x) \
  ({ int *__l, __c, __d;						      \
     asm ("leal " #x "@tlsgd(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "nop"								      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d));			      \
     __l; })
# else
#  define TLS_GD(x) \
  ({ int *__l, __b, __c, __d;						      \
     asm ("call 1f\n\t"							      \
	  ".subsection 1\n"						      \
	  "1:\tmovl (%%esp), %%ebx\n\t"					      \
	  "ret\n\t"							      \
	  ".previous\n\t"						      \
	  "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"			      \
	  "leal " #x "@tlsgd(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "nop"								      \
	  : "=a" (__l), "=&b" (__b), "=&c" (__c), "=&d" (__d));		      \
     __l; })
# endif

#elif defined __x86_64__

# define TLS_LE(x) \
  ({ int *__l;								      \
     asm ("mov %%fs:0,%0\n\t"						      \
	  "lea " #x "@tpoff(%0), %0"					      \
	  : "=r" (__l));						      \
     __l; })

# define TLS_IE(x) \
  ({ int *__l;								      \
     asm ("mov %%fs:0,%0\n\t"						      \
	  "add " #x "@gottpoff(%%rip),%0"				      \
	  : "=r" (__l));						      \
     __l; })

# define TLS_LD(x) \
  ({ int *__l, __c, __d;						      \
     asm ("leaq " #x "@tlsld(%%rip),%%rdi\n\t"				      \
	  "call __tls_get_addr@plt\n\t"					      \
	  "leaq " #x "@dtpoff(%%rax), %%rax"				      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d)			      \
	  : : "rdi", "rsi", "r8", "r9", "r10", "r11"); 			      \
     __l; })

# ifdef __ILP32__
#  define TLS_GD_PREFIX
# else
#  define TLS_GD_PREFIX	".byte 0x66\n\t"
# endif

# define TLS_GD(x) \
  ({ int *__l, __c, __d;						      \
     asm (TLS_GD_PREFIX							      \
	  "leaq " #x "@tlsgd(%%rip),%%rdi\n\t"				      \
	  ".word 0x6666\n\t"						      \
	  "rex64\n\t"							      \
	  "call __tls_get_addr@plt"					      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d)			      \
	  : : "rdi", "rsi", "r8", "r9", "r10", "r11"); 			      \
     __l; })

#elif defined __sh__

# define TLS_LE(x) \
  ({ int *__l; void *__tp;						      \
     asm ("stc gbr,%1\n\t"						      \
	  "mov.l 1f,%0\n\t"						      \
	  "bra 2f\n\t"							      \
	  " add %1,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tpoff\n\t"					      \
	  "2:"								      \
	  : "=r" (__l), "=r" (__tp));					      \
     __l; })

# ifdef PIC
#  define TLS_IE(x) \
  ({ int *__l; void *__tp;						      \
     register void *__gp __asm__("r12");				      \
     asm ("mov.l 1f,r0\n\t"						      \
	  "stc gbr,%1\n\t"						      \
	  "mov.l @(r0,r12),%0\n\t"					      \
	  "bra 2f\n\t"							      \
	  " add %1,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@gottpoff\n\t"				      \
	  "2:"								      \
	  : "=r" (__l), "=r" (__tp) : "r" (__gp) : "r0");		      \
     __l; })
# else
#  define TLS_IE(x) \
  ({ int *__l; void *__tp;						      \
     asm ("mov.l r12,@-r15\n\t"						      \
	  "mova 0f,r0\n\t"						      \
	  "mov.l 0f,r12\n\t"						      \
	  "add r0,r12\n\t"						      \
	  "mov.l 1f,r0\n\t"						      \
	  "stc gbr,%1\n\t"						      \
	  "mov.l @(r0,r12),%0\n\t"					      \
	  "bra 2f\n\t"							      \
	  " add %1,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@gottpoff\n\t"				      \
	  "0: .long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  "2: mov.l @r15+,r12"						      \
	  : "=r" (__l), "=r" (__tp) : : "r0");				      \
     __l; })
#endif

# ifdef PIC
#  define TLS_LD(x) \
  ({ int *__l;								      \
     register void *__gp __asm__("r12");				      \
     asm ("mov.l 1f,r4\n\t"						      \
	  "mova 2f,r0\n\t"						      \
	  "mov.l 2f,r1\n\t"						      \
	  "add r0,r1\n\t"						      \
	  "jsr @r1\n\t"							      \
	  " add r12,r4\n\t"						      \
	  "bra 4f\n\t"							      \
	  " nop\n\t"							      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tlsldm\n\t"					      \
	  "2: .long __tls_get_addr@plt\n\t"				      \
	  "4: mov.l 3f,%0\n\t"						      \
	  "bra 5f\n\t"							      \
	  " add r0,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "3: .long " #x "@dtpoff\n\t"					      \
	  "5:"								      \
	  : "=r" (__l) : "r" (__gp) : "r0", "r1", "r2", "r3", "r4", "r5",     \
				      "r6", "r7", "pr", "t");		      \
     __l; })
# else
#  define TLS_LD(x) \
  ({ int *__l;								      \
     asm ("mov.l r12,@-r15\n\t"						      \
	  "mova 0f,r0\n\t"						      \
	  "mov.l 0f,r12\n\t"						      \
	  "add r0,r12\n\t"						      \
	  "mov.l 1f,r4\n\t"						      \
	  "mova 2f,r0\n\t"						      \
	  "mov.l 2f,r1\n\t"						      \
	  "add r0,r1\n\t"						      \
	  "jsr @r1\n\t"							      \
	  " add r12,r4\n\t"						      \
	  "bra 4f\n\t"							      \
	  " nop\n\t"							      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tlsldm\n\t"					      \
	  "2: .long __tls_get_addr@plt\n\t"				      \
	  "0: .long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  "4: mov.l 3f,%0\n\t"						      \
	  "bra 5f\n\t"							      \
	  " add r0,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "3: .long " #x "@dtpoff\n\t"					      \
	  "5: mov.l @r15+,r12"						      \
	  : "=r" (__l) : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",    \
			   "pr", "t");					      \
     __l; })
#endif

# ifdef PIC
#  define TLS_GD(x) \
  ({ int *__l;								      \
     register void *__gp __asm__("r12");				      \
     asm ("mov.l 1f,r4\n\t"						      \
	  "mova 2f,r0\n\t"						      \
	  "mov.l 2f,r1\n\t"						      \
	  "add r0,r1\n\t"						      \
	  "jsr @r1\n\t"							      \
	  " add r12,r4\n\t"						      \
	  "bra 3f\n\t"							      \
	  " mov r0,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tlsgd\n\t"					      \
	  "2: .long __tls_get_addr@plt\n\t"				      \
	  "3:"								      \
	  : "=r" (__l) : "r" (__gp) : "r0", "r1", "r2", "r3", "r4", "r5",     \
				      "r6", "r7", "pr", "t");		      \
     __l; })
# else
#  define TLS_GD(x) \
  ({ int *__l;								      \
     asm ("mov.l r12,@-r15\n\t"						      \
	  "mova 0f,r0\n\t"						      \
	  "mov.l 0f,r12\n\t"						      \
	  "add r0,r12\n\t"						      \
	  "mov.l 1f,r4\n\t"						      \
	  "mova 2f,r0\n\t"						      \
	  "mov.l 2f,r1\n\t"						      \
	  "add r0,r1\n\t"						      \
	  "jsr @r1\n\t"							      \
	  " add r12,r4\n\t"						      \
	  "bra 3f\n\t"							      \
	  " mov r0,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tlsgd\n\t"					      \
	  "2: .long __tls_get_addr@plt\n\t"				      \
	  "0: .long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  "3: mov.l @r15+,r12"						      \
	  : "=r" (__l) : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",    \
			   "pr", "t");					      \
     __l; })
#endif

#elif defined __sparc__ && !defined __arch64__

# define TLS_LE(x) \
  ({ int *__l;								      \
     asm ("sethi %%tle_hix22(" #x "), %0" : "=r" (__l));		      \
     asm ("xor %1, %%tle_lox10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     asm ("add %%g7, %1, %0" : "=r" (__l) : "r" (__l));			      \
     __l; })

# ifdef __PIC__
#  define TLS_LOAD_PIC \
  ({ register long pc __asm__ ("%o7");					      \
     long got;								      \
     asm ("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t"			      \
	  "call .+8\n\t"						      \
	  "add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n\t"		      \
	  "add %1, %0, %1\n\t"						      \
	  : "=r" (pc), "=r" (got));					      \
     got; })
# else
#  define TLS_LOAD_PIC \
   ({ long got;								      \
      asm (".hidden _GLOBAL_OFFSET_TABLE_\n\t"				      \
	   "sethi %%hi(_GLOBAL_OFFSET_TABLE_), %0\n\t"			      \
	   "or %0, %%lo(_GLOBAL_OFFSET_TABLE_), %0"			      \
	   : "=r" (got));						      \
      got; })
# endif

# define TLS_IE(x) \
  ({ int *__l;								      \
     asm ("sethi %%tie_hi22(" #x "), %0" : "=r" (__l));			      \
     asm ("add %1, %%tie_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     asm ("ld [%1 + %2], %0, %%tie_ld(" #x ")"				      \
	  : "=r" (__l) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     asm ("add %%g7, %1, %0, %%tie_add(" #x ")" : "=r" (__l) : "r" (__l));    \
     __l; })

# define TLS_LD(x) \
  ({ int *__l; register void *__o0 asm ("%o0");				      \
     long __o;								      \
     asm ("sethi %%tldm_hi22(" #x "), %0" : "=r" (__l));		      \
     asm ("add %1, %%tldm_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     asm ("add %1, %2, %0, %%tldm_add(" #x ")"				      \
	  : "=r" (__o0) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     asm ("call __tls_get_addr, %%tgd_call(" #x ")\n\t"			      \
	  " nop"							      \
	  : "=r" (__o0) : "0" (__o0)					      \
	  : "g1", "g2", "g3", "g4", "g5", "g6", "o1", "o2", "o3", "o4",	      \
	    "o5", "o7", "cc");						      \
     asm ("sethi %%tldo_hix22(" #x "), %0" : "=r" (__o));		      \
     asm ("xor %1, %%tldo_lox10(" #x "), %0" : "=r" (__o) : "r" (__o));	      \
     asm ("add %1, %2, %0, %%tldo_add(" #x ")" : "=r" (__l)		      \
	  : "r" (__o0), "r" (__o));					      \
     __l; })

# define TLS_GD(x) \
  ({ int *__l; register void *__o0 asm ("%o0");				      \
     asm ("sethi %%tgd_hi22(" #x "), %0" : "=r" (__l));			      \
     asm ("add %1, %%tgd_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     asm ("add %1, %2, %0, %%tgd_add(" #x ")"				      \
	  : "=r" (__o0) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     asm ("call __tls_get_addr, %%tgd_call(" #x ")\n\t"			      \
	  " nop"							      \
	  : "=r" (__o0) : "0" (__o0)					      \
	  : "g1", "g2", "g3", "g4", "g5", "g6", "o1", "o2", "o3", "o4",	      \
	    "o5", "o7", "cc");						      \
     __o0; })

#elif defined __sparc__ && defined __arch64__

# define TLS_LE(x) \
  ({ int *__l;								      \
     asm ("sethi %%tle_hix22(" #x "), %0" : "=r" (__l));		      \
     asm ("xor %1, %%tle_lox10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     asm ("add %%g7, %1, %0" : "=r" (__l) : "r" (__l));			      \
     __l; })

# ifdef __PIC__
#  define TLS_LOAD_PIC \
  ({ long pc, got;							      \
     asm ("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t"			      \
	  "rd %%pc, %0\n\t"						      \
	  "add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n\t"		      \
	  "add %1, %0, %1\n\t"						      \
	  : "=r" (pc), "=r" (got));					      \
     got; })
# else
#  define TLS_LOAD_PIC \
   ({ long got;								      \
      asm (".hidden _GLOBAL_OFFSET_TABLE_\n\t"				      \
	   "sethi %%hi(_GLOBAL_OFFSET_TABLE_), %0\n\t"			      \
	   "or %0, %%lo(_GLOBAL_OFFSET_TABLE_), %0"			      \
	   : "=r" (got));						      \
      got; })
# endif

# define TLS_IE(x) \
  ({ int *__l;								      \
     asm ("sethi %%tie_hi22(" #x "), %0" : "=r" (__l));			      \
     asm ("add %1, %%tie_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     asm ("ldx [%1 + %2], %0, %%tie_ldx(" #x ")"			      \
	  : "=r" (__l) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     asm ("add %%g7, %1, %0, %%tie_add(" #x ")" : "=r" (__l) : "r" (__l));    \
     __l; })

# define TLS_LD(x) \
  ({ int *__l; register void *__o0 asm ("%o0");				      \
     long __o;								      \
     asm ("sethi %%tldm_hi22(" #x "), %0" : "=r" (__l));		      \
     asm ("add %1, %%tldm_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     asm ("add %1, %2, %0, %%tldm_add(" #x ")"				      \
	  : "=r" (__o0) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     asm ("call __tls_get_addr, %%tgd_call(" #x ")\n\t"			      \
	  " nop"							      \
	  : "=r" (__o0) : "0" (__o0)					      \
	  : "g1", "g2", "g3", "g4", "g5", "g6", "o1", "o2", "o3", "o4",	      \
	    "o5", "o7", "cc");						      \
     asm ("sethi %%tldo_hix22(" #x "), %0" : "=r" (__o));		      \
     asm ("xor %1, %%tldo_lox10(" #x "), %0" : "=r" (__o) : "r" (__o));	      \
     asm ("add %1, %2, %0, %%tldo_add(" #x ")" : "=r" (__l)		      \
	  : "r" (__o0), "r" (__o));					      \
     __l; })

# define TLS_GD(x) \
  ({ int *__l; register void *__o0 asm ("%o0");				      \
     asm ("sethi %%tgd_hi22(" #x "), %0" : "=r" (__l));			      \
     asm ("add %1, %%tgd_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     asm ("add %1, %2, %0, %%tgd_add(" #x ")"				      \
	  : "=r" (__o0) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     asm ("call __tls_get_addr, %%tgd_call(" #x ")\n\t"			      \
	  " nop"							      \
	  : "=r" (__o0) : "0" (__o0)					      \
	  : "g1", "g2", "g3", "g4", "g5", "g6", "o1", "o2", "o3", "o4",	      \
	    "o5", "o7", "cc");						      \
     __o0; })

#elif defined __s390x__

# define TLS_LE(x) \
  ({ unsigned long __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@ntpoff\n"					      \
	  "1:\tlg %0,0(%0)"						      \
	  : "=a" (__offset) : : "cc" );					      \
     (int *) (__builtin_thread_pointer() + __offset); })

# ifdef PIC
#  define TLS_IE(x) \
  ({ unsigned long __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@gotntpoff\n"				      \
	  "1:\tlg %0,0(%0)\n\t"						      \
	  "lg %0,0(%0,%%r12):tls_load:" #x				      \
	  : "=&a" (__offset) : : "cc" );				      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_IE(x) \
  ({ unsigned long  __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@indntpoff\n"				      \
	  "1:\t lg %0,0(%0)\n\t"					      \
	  "lg %0,0(%0):tls_load:" #x					      \
	  : "=&a" (__offset) : : "cc" );				      \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

# ifdef PIC
#  define TLS_LD(x) \
  ({ unsigned long __offset, __save12;					      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@tlsldm\n\t"					      \
	  ".quad " #x "@dtpoff\n"					      \
	  "1:\tlgr %1,%%r12\n\t"					      \
	  "larl %%r12,_GLOBAL_OFFSET_TABLE_\n\t"			      \
	  "lg %%r2,0(%0)\n\t"						      \
	  "brasl %%r14,__tls_get_offset@plt:tls_ldcall:" #x "\n\t"	      \
	  "lg %0,8(%0)\n\t"						      \
	  "algr %0,%%r2\n\t"						      \
	  "lgr %%r12,%1"						      \
	  : "=&a" (__offset), "=&a" (__save12)				      \
	  : : "cc", "0", "1", "2", "3", "4", "5", "14" );		      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_LD(x) \
  ({ unsigned long __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@tlsldm\n\t"					      \
	  ".quad " #x "@dtpoff\n"					      \
	  "1:\tlarl %%r12,_GLOBAL_OFFSET_TABLE_\n\t"			      \
	  "lg %%r2,0(%0)\n\t"						      \
	  "brasl %%r14,__tls_get_offset@plt:tls_ldcall:" #x "\n\t"	      \
	  "lg %0,8(%0)\n\t"						      \
	  "algr %0,%%r2"						      \
	  : "=&a" (__offset)						      \
	  : : "cc", "0", "1", "2", "3", "4", "5", "12", "14" );		      \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

# ifdef PIC
#  define TLS_GD(x) \
  ({ unsigned long __offset, __save12;					      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@tlsgd\n"					      \
	  "1:\tlgr %1,%%r12\n\t"					      \
	  "larl %%r12,_GLOBAL_OFFSET_TABLE_\n\t"			      \
	  "lg %%r2,0(%0)\n\t"						      \
	  "brasl %%r14,__tls_get_offset@plt:tls_gdcall:" #x "\n\t"	      \
	  "lgr %0,%%r2\n\t"						      \
	  "lgr %%r12,%1"						      \
	  : "=&a" (__offset), "=&a" (__save12)				      \
	  : : "cc", "0", "1", "2", "3", "4", "5", "14" );		      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_GD(x) \
  ({ unsigned long __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@tlsgd\n"					      \
	  "1:\tlarl %%r12,_GLOBAL_OFFSET_TABLE_\n\t"			      \
	  "lg %%r2,0(%0)\n\t"						      \
	  "brasl %%r14,__tls_get_offset@plt:tls_gdcall:" #x "\n\t"	      \
	  "lgr %0,%%r2"							      \
	  : "=&a" (__offset)						      \
	  : : "cc", "0", "1", "2", "3", "4", "5", "12", "14" );		      \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

#elif defined __s390__

# define TLS_LE(x) \
  ({ unsigned long __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.long " #x "@ntpoff\n"					      \
	  "1:\tl %0,0(%0)"						      \
	  : "=a" (__offset) : : "cc" );					      \
     (int *) (__builtin_thread_pointer() + __offset); })

# ifdef PIC
#  define TLS_IE(x) \
  ({ unsigned long __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.long " #x "@gotntpoff\n"				      \
	  "1:\tl %0,0(%0)\n\t"						      \
	  "l %0,0(%0,%%r12):tls_load:" #x				      \
	  : "=&a" (__offset) : : "cc" );				      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_IE(x) \
  ({ unsigned long  __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.long " #x "@indntpoff\n"				      \
	  "1:\t l %0,0(%0)\n\t"						      \
	  "l %0,0(%0):tls_load:" #x					      \
	  : "=&a" (__offset) : : "cc" );				      \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

# ifdef PIC
#  define TLS_LD(x) \
  ({ unsigned long __offset, __save12;					      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.long _GLOBAL_OFFSET_TABLE_-0b\n\t"			      \
	  ".long __tls_get_offset@plt-0b\n\t"				      \
	  ".long " #x "@tlsldm\n\t"					      \
	  ".long " #x "@dtpoff\n"					      \
	  "1:\tlr %1,%%r12\n\t"						      \
	  "l %%r12,0(%0)\n\t"						      \
	  "la %%r12,0(%%r12,%0)\n\t"					      \
	  "l %%r1,4(%0)\n\t"						      \
	  "l %%r2,8(%0)\n\t"						      \
	  "bas %%r14,0(%%r1,%0):tls_ldcall:" #x "\n\t"			      \
	  "l %0,12(%0)\n\t"						      \
	  "alr %0,%%r2\n\t"						      \
	  "lr %%r12,%1"							      \
	  : "=&a" (__offset), "=&a" (__save12)				      \
	  : : "cc", "0", "1", "2", "3", "4", "5" );			      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_LD(x) \
  ({ unsigned long __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  ".long __tls_get_offset@plt\n\t"				      \
	  ".long " #x "@tlsldm\n\t"					      \
	  ".long " #x "@dtpoff\n"					      \
	  "1:\tl %%r12,0(%0)\n\t"					      \
	  "l %%r1,4(%0)\n\t"						      \
	  "l %%r2,8(%0)\n\t"						      \
	  "bas %%r14,0(%%r1):tls_ldcall:" #x "\n\t"			      \
	  "l %0,12(%0)\n\t"						      \
	  "alr %0,%%r2"							      \
	  : "=&a" (__offset) : : "cc", "0", "1", "2", "3", "4", "5", "12" );  \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

# ifdef PIC
#  define TLS_GD(x) \
  ({ unsigned long __offset, __save12;					      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.long _GLOBAL_OFFSET_TABLE_-0b\n\t"			      \
	  ".long __tls_get_offset@plt-0b\n\t"				      \
	  ".long " #x "@tlsgd\n"					      \
	  "1:\tlr %1,%%r12\n\t"						      \
	  "l %%r12,0(%0)\n\t"						      \
	  "la %%r12,0(%%r12,%0)\n\t"					      \
	  "l %%r1,4(%0)\n\t"						      \
	  "l %%r2,8(%0)\n\t"						      \
	  "bas %%r14,0(%%r1,%0):tls_gdcall:" #x "\n\t"			      \
	  "lr %0,%%r2\n\t"						      \
	  "lr %%r12,%1"							      \
	  : "=&a" (__offset), "=&a" (__save12)				      \
	  : : "cc", "0", "1", "2", "3", "4", "5" );			      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_GD(x) \
  ({ unsigned long __offset;						      \
     asm ("bras %0,1f\n"						      \
	  "0:\t.long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  ".long __tls_get_offset@plt\n\t"				      \
	  ".long " #x "@tlsgd\n"					      \
	  "1:\tl %%r12,0(%0)\n\t"					      \
	  "l %%r1,4(%0)\n\t"						      \
	  "l %%r2,8(%0)\n\t"						      \
	  "bas %%r14,0(%%r1):tls_gdcall:" #x "\n\t"			      \
	  "lr %0,%%r2"							      \
	  : "=&a" (__offset) : : "cc", "0", "1", "2", "3", "4", "5", "12" );  \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

#elif defined __powerpc__

# define __TLS_CALL_CLOBBERS						      \
	"0", "4", "5", "6", "7", "8", "9", "10", "11", "12",		      \
	"lr", "ctr", "cr0", "cr1", "cr5", "cr6", "cr7"

# ifndef __powerpc64__

#  include "config.h"

/* PowerPC32 Local Exec TLS access.  */
#  define TLS_LE(x)							      \
  ({ int *__result;							      \
     asm ("addi %0,2," #x "@tprel"					      \
	  : "=r" (__result));						      \
     __result; })

/* PowerPC32 Initial Exec TLS access.  */
#  ifdef HAVE_ASM_PPC_REL16
#   define TLS_IE(x)							      \
  ({ int *__result;							      \
     asm ("bcl 20,31,1f\n1:\t"						      \
	  "mflr %0\n\t"							      \
	  "addis %0,%0,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"			      \
	  "addi %0,%0,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"			      \
	  "lwz %0," #x "@got@tprel(%0)\n\t"				      \
	  "add %0,%0," #x "@tls"					      \
	  : "=b" (__result) :						      \
	  : "lr");							      \
     __result; })
#  else
#   define TLS_IE(x)							      \
  ({ int *__result;							      \
     asm ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"			      \
	  "mflr %0\n\t"							      \
	  "lwz %0," #x "@got@tprel(%0)\n\t"				      \
	  "add %0,%0," #x "@tls"					      \
	  : "=b" (__result) :						      \
	  : "lr");							      \
     __result; })
#  endif

/* PowerPC32 Local Dynamic TLS access.  */
#  ifdef HAVE_ASM_PPC_REL16
#   define TLS_LD(x)							      \
  ({ int *__result;							      \
     asm ("bcl 20,31,1f\n1:\t"						      \
	  "mflr 3\n\t"							      \
	  "addis 3,3,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"			      \
	  "addi 3,3,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"			      \
	  "addi 3,3," #x "@got@tlsld\n\t"				      \
	  "bl __tls_get_addr@plt\n\t"					      \
	  "addi %0,3," #x "@dtprel"					      \
	  : "=r" (__result) :						      \
	  : "3", __TLS_CALL_CLOBBERS);					      \
     __result; })
#  else
#   define TLS_LD(x)							      \
  ({ int *__result;							      \
     asm ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"			      \
	  "mflr 3\n\t"							      \
	  "addi 3,3," #x "@got@tlsld\n\t"				      \
	  "bl __tls_get_addr@plt\n\t"					      \
	  "addi %0,3," #x "@dtprel"					      \
	  : "=r" (__result) :						      \
	  : "3", __TLS_CALL_CLOBBERS);					      \
     __result; })
#  endif

/* PowerPC32 General Dynamic TLS access.  */
#  ifdef HAVE_ASM_PPC_REL16
#   define TLS_GD(x)							      \
  ({ register int *__result __asm__ ("r3");				      \
     asm ("bcl 20,31,1f\n1:\t"						      \
	  "mflr 3\n\t"							      \
	  "addis 3,3,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"			      \
	  "addi 3,3,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"			      \
	  "addi 3,3," #x "@got@tlsgd\n\t"				      \
	  "bl __tls_get_addr@plt"					      \
	  : "=r" (__result) :						      \
	  : __TLS_CALL_CLOBBERS);					      \
     __result; })
#  else
#   define TLS_GD(x)							      \
  ({ register int *__result __asm__ ("r3");				      \
     asm ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"			      \
	  "mflr 3\n\t"							      \
	  "addi 3,3," #x "@got@tlsgd\n\t"				      \
	  "bl __tls_get_addr@plt"					      \
	  : "=r" (__result) :						      \
	  : __TLS_CALL_CLOBBERS);					      \
     __result; })
#  endif

# else

/* PowerPC64 Local Exec TLS access.  */
#  define TLS_LE(x)							      \
  ({ int * __result;							      \
     asm ("addis %0,13," #x "@tprel@ha\n\t"				      \
	  "addi  %0,%0," #x "@tprel@l"					      \
	  : "=b" (__result) );						      \
     __result;								      \
  })
/* PowerPC64 Initial Exec TLS access.  */
#  define TLS_IE(x)							      \
  ({ int * __result;							      \
     asm ("ld  %0," #x "@got@tprel(2)\n\t"				      \
	  "add %0,%0," #x "@tls"					      \
	  : "=r" (__result) );						      \
     __result;								      \
  })
#  ifdef HAVE_ASM_GLOBAL_DOT_NAME
#   define __TLS_GET_ADDR ".__tls_get_addr"
#  else
#   define __TLS_GET_ADDR "__tls_get_addr"
#  endif
/* PowerPC64 Local Dynamic TLS access.  */
#  define TLS_LD(x)							      \
  ({ int * __result;							      \
     asm ("addi  3,2," #x "@got@tlsld\n\t"				      \
	  "bl    " __TLS_GET_ADDR "\n\t"				      \
	  "nop   \n\t"							      \
	  "addis %0,3," #x "@dtprel@ha\n\t"				      \
	  "addi  %0,%0," #x "@dtprel@l"					      \
	  : "=b" (__result) :						      \
	  : "3", __TLS_CALL_CLOBBERS);					      \
     __result;								      \
  })
/* PowerPC64 General Dynamic TLS access.  */
#  define TLS_GD(x)							      \
  ({ register int *__result __asm__ ("r3");				      \
     asm ("addi  3,2," #x "@got@tlsgd\n\t"				      \
	  "bl    " __TLS_GET_ADDR "\n\t"				      \
	  "nop   "							      \
	  : "=r" (__result) :						      \
	  : __TLS_CALL_CLOBBERS);					      \
     __result;								      \
  })
# endif

#elif !defined TLS_LE || !defined TLS_IE \
      || !defined TLS_LD || !defined TLS_GD
# error "No support for this architecture so far."
#endif
