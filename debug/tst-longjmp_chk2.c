/* Test case mostly written by Paolo Bonzini <pbonzini@redhat.com>.  */
#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>


static jmp_buf mainloop;
static sigset_t mainsigset;
static int pass;


static void
stackoverflow_handler (int sig)
{
  stack_t altstack;
  pass++;
  sigaltstack (NULL, &altstack);
  /* Using printf is not really kosher in signal handlers but we know
     it will work.  */
  printf ("%*sin signal handler\n", pass, "");
  if (altstack.ss_flags & SS_ONSTACK)
    printf ("%*son alternate stack\n", pass, "");
  siglongjmp (mainloop, pass);
}


static volatile int *
recurse_1 (int n, volatile int *p)
{
  if (n >= 0)
    *recurse_1 (n + 1, p) += n;
  return p;
}


static int
recurse (int n)
{
  int sum = 0;
  return *recurse_1 (n, &sum);
}


static int
do_test (void)
{
  char mystack[SIGSTKSZ];
  stack_t altstack;
  struct sigaction action;
  sigset_t emptyset;
  /* Before starting the endless recursion, try to be friendly to the user's
     machine.  On some Linux 2.2.x systems, there is no stack limit for user
     processes at all.  We don't want to kill such systems.  */
  struct rlimit rl;
  rl.rlim_cur = rl.rlim_max = 0x100000; /* 1 MB */
  setrlimit (RLIMIT_STACK, &rl);
  /* Install the alternate stack.  */
  altstack.ss_sp = mystack;
  altstack.ss_size = sizeof (mystack);
  altstack.ss_flags = 0; /* no SS_DISABLE */
  if (sigaltstack (&altstack, NULL) < 0)
    {
      puts ("first sigaltstack failed");
      return 0;
    }
  /* Install the SIGSEGV handler.  */
  sigemptyset (&action.sa_mask);
  action.sa_handler = &stackoverflow_handler;
  action.sa_flags = SA_ONSTACK;
  sigaction (SIGSEGV, &action, (struct sigaction *) NULL);
  sigaction (SIGBUS, &action, (struct sigaction *) NULL);

  /* Save the current signal mask.  */
  sigemptyset (&emptyset);
  sigprocmask (SIG_BLOCK, &emptyset, &mainsigset);

  /* Provoke two stack overflows in a row.  */
  if (sigsetjmp (mainloop, 1) != 0)
    {
      assert (pass != 0);
      printf ("%*sout of signal handler\n", pass, "");
    }
  else
    assert (pass == 0);

  sigaltstack (NULL, &altstack);
  if (altstack.ss_flags & SS_ONSTACK)
    printf ("%*son alternate stack\n", pass, "");
  else
    printf ("%*snot on alternate stack\n", pass, "");

  if (pass < 2)
    {
      recurse (0);
      puts ("recurse call returned");
      return 2;
    }

  altstack.ss_flags |= SS_DISABLE;
  if (sigaltstack (&altstack, NULL) == -1)
    printf ("disabling alternate stack failed\n");
  else
    printf ("disabling alternate stack succeeded \n");

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
