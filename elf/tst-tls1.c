/* glibc test for TLS in ld.so.  */
#include <stdio.h>

#include <tls.h>


/* XXX Until gcc gets told how to define and use thread-local
   variables we will have to resort to use asms.  */
asm (".tls_common foo,4,4");
asm (".tls_common bar,4,4");


int
main (void)
{
#ifdef USE_TLS
  int result = 0;
  int *ap, *bp;

  /* XXX Each architecture must have its own asm for now.  */
# ifdef __i386__
#  define TLS_LE(x) \
  ({ int *__l;								      \
     asm ("movl %%gs:0,%0\n\t"						      \
	  "subl $" #x "@tpoff,%0"					      \
	  : "=r" (__l));						      \
     __l; })

#define TLS_IE(x) \
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

#define TLS_LD(x) \
  ({ int *__l, __b;							      \
     asm ("call 1f\n\t"							      \
	  ".subsection 1\n"						      \
	  "1:\tmovl (%%esp), %%ebx\n\t"					      \
	  "ret\n\t"							      \
	  ".previous\n\t"						      \
	  "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"			      \
	  "leal " #x "@tlsldm(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "leal " #x "@dtpoff(%%eax), %%eax"				      \
	  : "=a" (__l), "=&b" (__b));					      \
     __l; })

#define TLS_GD(x) \
  ({ int *__l, __b;							      \
     asm ("call 1f\n\t"							      \
	  ".subsection 1\n"						      \
	  "1:\tmovl (%%esp), %%ebx\n\t"					      \
	  "ret\n\t"							      \
	  ".previous\n\t"						      \
	  "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"			      \
	  "leal " #x "@tlsgd(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "nop"								      \
	  : "=a" (__l), "=&b" (__b));					      \
     __l; })

# else
#  error "No support for this architecture so far."
# endif

  /* Set the variable using the local exec model.  */
  puts ("set bar to 1 (LE)");
  ap = TLS_LE (bar);
  *ap = 1;


  /* Get variables using initial exec model.  */
  fputs ("get sum of foo and bar (IE)", stdout);
  ap = TLS_IE (foo);
  bp = TLS_IE (bar);
  printf (" = %d\n", *ap + *bp);
  result |= *ap + *bp != 1;
  if (*ap != 0)
    {
      printf ("foo = %d\n", *ap);
      result = 1;
    }
  if (*bp != 1)
    {
      printf ("bar = %d\n", *bp);
      result = 1;
    }


  /* Get variables using local dynamic model.  */
  fputs ("get sum of foo and bar (LD)", stdout);
  ap = TLS_LD (foo);
  bp = TLS_LD (bar);
  printf (" = %d\n", *ap + *bp);
  result |= *ap + *bp != 1;
  if (*ap != 0)
    {
      printf ("foo = %d\n", *ap);
      result = 1;
    }
  if (*bp != 1)
    {
      printf ("bar = %d\n", *bp);
      result = 1;
    }


  /* Get variables using generic dynamic model.  */
  fputs ("get sum of foo and bar (GD)", stdout);
  ap = TLS_GD (foo);
  bp = TLS_GD (bar);
  printf (" = %d\n", *ap + *bp);
  result |= *ap + *bp != 1;
  if (*ap != 0)
    {
      printf ("foo = %d\n", *ap);
      result = 1;
    }
  if (*bp != 1)
    {
      printf ("bar = %d\n", *bp);
      result = 1;
    }

  return result;
#else
  return 0;
#endif
}
