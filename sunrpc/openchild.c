/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * Open two pipes to a child process, one for reading, one for writing.
 * The pipes are accessed by FILE pointers. This is NOT a public
 * interface, but for internal use only!
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/clnt.h>

/*
 * returns pid, or -1 for failure
 */
int
_openchild (char *command, FILE ** fto, FILE ** ffrom)
{
  int i;
  int pid;
  int pdto[2];
  int pdfrom[2];

  if (pipe (pdto) < 0)
    goto error1;
  if (pipe (pdfrom) < 0)
    goto error2;
  switch (pid = fork ())
    {
    case -1:
      goto error3;

    case 0:
      /*
       * child: read from pdto[0], write into pdfrom[1]
       */
      close (0);
      dup (pdto[0]);
      close (1);
      dup (pdfrom[1]);
      fflush (stderr);
      for (i = _rpc_dtablesize () - 1; i >= 3; i--)
	close (i);
      fflush (stderr);
      execlp (command, command, 0);
      perror ("exec");
      _exit (~0);

    default:
      /*
       * parent: write into pdto[1], read from pdfrom[0]
       */
      *fto = fdopen (pdto[1], "w");
      close (pdto[0]);
      *ffrom = fdopen (pdfrom[0], "r");
      close (pdfrom[1]);
      break;
    }
  return pid;

  /*
   * error cleanup and return
   */
error3:
  close (pdfrom[0]);
  close (pdfrom[1]);
error2:
  close (pdto[0]);
  close (pdto[1]);
error1:
  return -1;
}
