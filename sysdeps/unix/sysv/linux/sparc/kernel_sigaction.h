/* Linux/SPARC version.  This is the sigaction struction from the Linux
   2.1.20 kernel.  */

struct sigaction
  {
    __sighandler_t sa_handler;
    sigset_t sa_mask;
    unsigned long int sa_flags;
    void (*sa_restorer) (void);     /* not used by Linux/SPARC yet */
  };

#define HAVE_SA_RESTORER
