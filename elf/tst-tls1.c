/* glibc test for TLS in ld.so.  */
#include <stdio.h>

#include <tls.h>


/* XXX Until gcc gets told how to define and use thread-local
   variables we will have to resort to use asms.  */
//asm (".tls_common foo,4,4");
asm (".section \".tdata\", \"awT\", @progbits\n\t"
     ".align 4\n\t"
     ".globl foo\n"
     "foo:\t.long 0\n\t"
     ".globl bar\n"
     "bar:\t.long 0\n\t"
     ".previous");


int
main (void)
{
#ifdef USE_TLS
  int result = 0;
  int a, b;

  /* XXX Each architecture must have its own asm for now.  */
# ifdef __i386__
  /* Set the variable using the local exec model.  */
  puts ("set bar to 1 (LE)");
  asm ("movl %gs:0,%eax\n\t"
     "subl $bar@tpoff,%eax\n\t"
     "movl $1,(%eax)");

#if 0
  // XXX Doesn't work yet; no runtime relocations.
  fputs ("get sum of foo and bar (IE)", stdout);
  asm ("call 1f\n\t"
       ".subsection 1\n"
       "1:\tmovl (%%esp), %%ebx\n\t"
       "ret\n\t"
       ".previous\n\t"
       "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"
       "movl %%gs:0,%%eax\n\t"
       "movl %%eax,%%edx\n\t"
       "addl foo@gottpoff(%%ebx),%%eax\n\t"
       "addl bar@gottpoff(%%ebx),%%eax\n\t"
       "movl (%%eax), %0\n\t"
       "addl (%%edx), %0"
       : "=a" (a), "=&b" (b));
  printf (" = %d\n", a);
  result |= a != 1;
#endif

  fputs ("get sum of foo and bar (GD)", stdout);
  asm ("call 1f\n\t"
       ".subsection 1\n"
       "1:\tmovl (%%esp), %%ebx\n\t"
       "ret\n\t"
       ".previous\n\t"
       "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"
       "leal foo@tlsgd(%%ebx),%%eax\n\t"
       "call ___tls_get_addr@plt\n\t"
       "nop\n\t"
       "movl (%%eax), %%edx\n\t"
       "leal bar@tlsgd(%%ebx),%%eax\n\t"
       "call ___tls_get_addr@plt\n\t"
       "nop\n\t"
       "addl (%%eax), %%edx\n\t"
       : "=&a" (a), "=d" (b));
  printf (" = %d\n", b);
  result |= b != 1;

# else
#  error "No support for this architecture so far."
# endif

  return result;
#else
  return 0;
#endif
}
