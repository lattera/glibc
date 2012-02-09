/* Macros to support TLS testing in times of missing compiler support.  */

extern void *__tls_get_addr (void *);

# define TLS_GD(x)						\
  ({ void *__result;						\
     asm ("lda %0, " #x "($gp) !tlsgd" : "=r" (__result));	\
     __tls_get_addr (__result); })

# define TLS_LD(x)						\
  ({ void *__result;						\
     asm ("lda %0, " #x "($gp) !tlsldm" : "=r" (__result));	\
     __result = __tls_get_addr (__result);			\
     asm ("lda %0, " #x "(%0) !dtprel" : "+r" (__result));	\
     __result; })

# define TLS_IE(x)						\
  ({ long ofs;							\
     asm ("ldq %0, " #x "($gp) !gottprel" : "=r"(ofs));		\
     __builtin_thread_pointer () + ofs; })

# define TLS_LE(x)						\
  ({ void *__result = __builtin_thread_pointer ();		\
     asm ("lda %0, " #x "(%0) !tprel" : "+r" (__result));	\
     __result; })
