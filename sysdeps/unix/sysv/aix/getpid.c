/* This is a system call.  We only have to provide the wrapper.  */
int
__getpid (void)
{
  return getpid ();
}
