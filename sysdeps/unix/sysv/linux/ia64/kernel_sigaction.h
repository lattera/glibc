/* This is the sigaction structure from the Linux 3.2 kernel.  */
struct kernel_sigaction
{
  __sighandler_t k_sa_handler;
  unsigned long sa_flags;
  sigset_t sa_mask;               /* mask last for extensibility */
};
