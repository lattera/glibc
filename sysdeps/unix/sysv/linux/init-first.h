/* The job of this fragment it to find argc and friends for INIT.
   This is done in one of two ways: either in the stack context
   of program start, or having dlopen pass them in.  */

#define SYSDEP_CALL_INIT(NAME, INIT)					      \
void NAME (void *arg)							      \
{									      \
  int argc;								      \
  char **argv, **envp;							      \
  /* The next variable is only here to work around a bug in gcc <= 2.7.2.1.   \
     If the address would be taken inside the expression the optimizer	      \
     would try to be too smart and throws it away.  Grrr.  */		      \
  int *dummy_addr = &_dl_starting_up;					      \
									      \
  __libc_multiple_libcs = dummy_addr && !_dl_starting_up;		      \
									      \
  if (!__libc_multiple_libcs)						      \
    {									      \
      argc = (int) arg;							      \
      argv = (char **) &arg + 1;					      \
      envp = &argv[argc+1];						      \
    }									      \
  else									      \
    {									      \
      argc = (int) arg;							      \
      argv = ((char ***) &arg)[1];					      \
      envp = ((char ***) &arg)[2];					      \
    }									      \
									      \
  INIT (argc, argv, envp);						      \
}
