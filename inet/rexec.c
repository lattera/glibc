/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rexec.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <alloca.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int	rexecoptions;

int
rexec(ahost, rport, name, pass, cmd, fd2p)
	char **ahost;
	int rport;
	const char *name, *pass, *cmd;
	int *fd2p;
{
	struct sockaddr_in sin, sin2, from;
	struct hostent hostbuf, *hp;
	size_t hstbuflen;
	char *hsttmpbuf;
	u_short port;
	int s, timo = 1, s3;
	char c;
	int herr;

	hstbuflen = 1024;
	hsttmpbuf = __alloca (hstbuflen);
	while (__gethostbyname_r (*ahost, &hostbuf, hsttmpbuf, hstbuflen,
				  &hp, &herr) < 0)
	  if (herr != NETDB_INTERNAL || errno != ERANGE)
	    {
	      __set_h_errno (herr);
	      herror(*ahost);
	      return -1;
	    }
	  else
	    {
	      /* Enlarge the buffer.  */
	      hstbuflen *= 2;
	      hsttmpbuf = __alloca (hstbuflen);
	    }

	*ahost = hp->h_name;
	ruserpass(hp->h_name, &name, &pass);
retry:
	s = __socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("rexec: socket");
		return (-1);
	}
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = rport;
	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
	if (__connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		if (errno == ECONNREFUSED && timo <= 16) {
			(void) __close(s);
			__sleep(timo);
			timo *= 2;
			goto retry;
		}
		perror(hp->h_name);
		return (-1);
	}
	if (fd2p == 0) {
		(void) __write(s, "", 1);
		port = 0;
	} else {
		char num[32];
		int s2, sin2len;

		s2 = __socket(AF_INET, SOCK_STREAM, 0);
		if (s2 < 0) {
			(void) __close(s);
			return (-1);
		}
		listen(s2, 1);
		sin2len = sizeof (sin2);
		if (getsockname(s2, (struct sockaddr *)&sin2, &sin2len) < 0 ||
		  sin2len != sizeof (sin2)) {
			perror("getsockname");
			(void) __close(s2);
			goto bad;
		}
		port = ntohs((u_short)sin2.sin_port);
		(void) sprintf(num, "%u", port);
		(void) __write(s, num, strlen(num)+1);
		{ int len = sizeof (from);
		  s3 = accept(s2, (struct sockaddr *)&from, &len);
		  __close(s2);
		  if (s3 < 0) {
			perror("accept");
			port = 0;
			goto bad;
		  }
		}
		*fd2p = s3;
	}
	(void) __write(s, name, strlen(name) + 1);
	/* should public key encypt the password here */
	(void) __write(s, pass, strlen(pass) + 1);
	(void) __write(s, cmd, strlen(cmd) + 1);
	if (__read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad;
	}
	if (c != 0) {
		while (__read(s, &c, 1) == 1) {
			(void) __write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad;
	}
	return (s);
bad:
	if (port)
		(void) __close(*fd2p);
	(void) __close(s);
	return (-1);
}
