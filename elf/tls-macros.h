/* Macros to support TLS testing in times of missing compiler support.  */

#define COMMON_INT_DEF(x) \
  asm (".tls_common " #x ",4,4")
/* XXX Until we get compiler support we don't need declarations.  */
#define COMMON_INT_DECL(x)

/* XXX This definition will probably be machine specific, too.  */
#define VAR_INT_DEF(x) \
  asm (".section .tdata\n\t"						      \
       ".globl " #x "\n"						      \
       #x ":\t.long 0\n\t"						      \
       ".size " #x ",4\n\t"						      \
       ".previous")
/* XXX Until we get compiler support we don't need declarations.  */
#define VAR_INT_DECL(x)


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
     asm ("movq %%fs:0,%0\n\t"						      \
	  "leaq " #x "@tpoff(%0), %0"					      \
	  : "=r" (__l));						      \
     __l; })

# define TLS_IE(x) \
  ({ int *__l;								      \
     asm ("movq %%fs:0,%0\n\t"						      \
	  "addq " #x "@gottpoff(%%rip),%0"				      \
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

# define TLS_GD(x) \
  ({ int *__l, __c, __d;						      \
     asm (".byte 0x66\n\t"						      \
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

# define TLS_IE(x) \
  ({ int *__l; void *__tp;						      \
     asm ("mova 0f,r0\n\t"						      \
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
	  "2:"								      \
	  : "=r" (__l), "=r" (__tp) : : "r0", "r12");			      \
     __l; })

# define TLS_LD(x) \
  ({ int *__l;								      \
     asm ("mova 0f,r0\n\t"						      \
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
	  "5:"								      \
	  : "=r" (__l) : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",    \
			   "r12", "pr", "t");				      \
     __l; })

# define TLS_GD(x) \
  ({ int *__l;								      \
     asm ("mova 0f,r0\n\t"						      \
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
	  "3:"								      \
	  : "=r" (__l) : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",    \
			   "r12", "pr", "t");				      \
     __l; })

#else
# error "No support for this architecture so far."
#endif
