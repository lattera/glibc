#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdlib.h>
#include <unistd.h>

#include <not-cancel.h>

#include "pty-private.h"

#if HAVE_PT_CHOWN
/* Close all file descriptors except the one specified.  */
static void
close_all_fds (void)
{
  DIR *dir = __opendir ("/proc/self/fd");
  if (dir != NULL)
    {
      struct dirent64 *d;
      while ((d = __readdir64 (dir)) != NULL)
	if (isdigit (d->d_name[0]))
	  {
	    char *endp;
	    long int fd = strtol (d->d_name, &endp, 10);
	    if (*endp == '\0' && fd != PTY_FILENO && fd != dirfd (dir))
	      close_not_cancel_no_status (fd);
	  }

      __closedir (dir);

      int nullfd = open_not_cancel_2 (_PATH_DEVNULL, O_RDONLY);
      assert (nullfd == STDIN_FILENO);
      nullfd = open_not_cancel_2 (_PATH_DEVNULL, O_WRONLY);
      assert (nullfd == STDOUT_FILENO);
      __dup2 (STDOUT_FILENO, STDERR_FILENO);
    }
}
# define CLOSE_ALL_FDS() close_all_fds()
#endif

#include <sysdeps/unix/grantpt.c>
