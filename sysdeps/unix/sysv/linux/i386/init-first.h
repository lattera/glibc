/* The job of this fragment it to find argc and friends for INIT.
   This is done in one of two ways: either in the stack context
   of program start, or having dlopen pass them in.  */

#define SYSDEP_CALL_INIT(NAME, INIT)					      \
void NAME (void *arg)							      \
{									      \
  int argc;								      \
  char **argv, **envp;							      \
									      \
  __libc_multiple_libcs = &_dl_starting_up && !_dl_starting_up;		      \
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
