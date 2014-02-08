#ifdef __thumb2__
#define ARM_PC_OFFSET "4"
#else
#define ARM_PC_OFFSET "8"
#endif

#define TLS_LE(x)					\
  ({ int *__result;					\
     void *tp = __builtin_thread_pointer ();		\
     asm ("ldr %0, 1f; "				\
	  "add %0, %1, %0; "				\
	  "b 2f; "					\
	  ".align 2; "					\
	  "1: .word " #x "(tpoff); "			\
	  "2: "						\
	  : "=&r" (__result) : "r" (tp));		\
     __result; })

#ifdef __thumb2__
#define TLS_IE(x)					\
  ({ int *__result;					\
     void *tp = __builtin_thread_pointer ();		\
     asm ("ldr %0, 1f; "				\
	  "3: add %0, pc, %0;"				\
	  "ldr %0, [%0];"				\
	  "add %0, %1, %0; "				\
	  "b 2f; "					\
	  ".align 2; "					\
	  "1: .word " #x "(gottpoff) + (. - 3b - 4); "	\
	  "2: "						\
	  : "=&r" (__result) : "r" (tp));		\
     __result; })
#else
#define TLS_IE(x)					\
  ({ int *__result;					\
     void *tp = __builtin_thread_pointer ();		\
     asm ("ldr %0, 1f; "				\
	  "3: ldr %0, [pc, %0];"			\
	  "add %0, %1, %0; "				\
	  "b 2f; "					\
	  ".align 2; "					\
	  "1: .word " #x "(gottpoff) + (. - 3b - 8); "	\
	  "2: "						\
	  : "=&r" (__result) : "r" (tp));		\
     __result; })
#endif

#define TLS_LD(x)					\
  ({ char *__result;					\
     int __offset;					\
     extern void *__tls_get_addr (void *);		\
     asm ("ldr %0, 2f; "				\
	  "1: add %0, pc, %0; "				\
	  "b 3f; "					\
	  ".align 2; "					\
	  "2: .word " #x "(tlsldm) + (. - 1b - "ARM_PC_OFFSET"); "	\
	  "3: "						\
	  : "=r" (__result));				\
     __result = (char *)__tls_get_addr (__result);	\
     asm ("ldr %0, 1f; "				\
	  "b 2f; "					\
	  ".align 2; "					\
	  "1: .word " #x "(tlsldo); "			\
	  "2: "						\
	  : "=r" (__offset));				\
     (int *) (__result + __offset); })

#define TLS_GD(x)					\
  ({ int *__result;					\
     extern void *__tls_get_addr (void *);		\
     asm ("ldr %0, 2f; "				\
	  "1: add %0, pc, %0; "				\
	  "b 3f; "					\
	  ".align 2; "					\
	  "2: .word " #x "(tlsgd) + (. - 1b - "ARM_PC_OFFSET"); "	\
	  "3: "						\
	  : "=r" (__result));				\
     (int *)__tls_get_addr (__result); })
