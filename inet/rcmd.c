/*
 * Copyright (c) 1983, 1993, 1994
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
static char sccsid[] = "@(#)rcmd.c	8.3 (Berkeley) 3/26/94";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <alloca.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


int	__ivaliduser __P((FILE *, u_int32_t, const char *, const char *));
static int __icheckhost __P((u_int32_t, char *)) internal_function;

int
rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
	char **ahost;
	u_short rport;
	const char *locuser, *remuser, *cmd;
	int *fd2p;
{
	struct hostent hostbuf, *hp;
	size_t hstbuflen;
	char *tmphstbuf;
	struct sockaddr_in sin, from;
	fd_set reads;
	int32_t oldmask;
	pid_t pid;
	int s, lport, timo;
	char c;
	int herr;

	pid = getpid();

	hstbuflen = 1024;
	tmphstbuf = __alloca (hstbuflen);
	while (__gethostbyname_r (*ahost, &hostbuf, tmphstbuf, hstbuflen,
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
	      tmphstbuf = __alloca (hstbuflen);
	    }

	*ahost = hp->h_name;
	oldmask = sigblock(sigmask(SIGURG));
	for (timo = 1, lport = IPPORT_RESERVED - 1;;) {
		s = rresvport(&lport);
		if (s < 0) {
			if (errno == EAGAIN)
				(void)fprintf(stderr,
				    _("rcmd: socket: All ports in use\n"));
			else
				(void)fprintf(stderr, "rcmd: socket: %m\n");
			sigsetmask(oldmask);
			return -1;
		}
		fcntl(s, F_SETOWN, pid);
		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], &sin.sin_addr,
		      MIN (sizeof (sin.sin_addr), hp->h_length));
		sin.sin_port = rport;
		if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
			break;
		(void)close(s);
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
			(void)sleep(timo);
			timo *= 2;
			continue;
		}
		if (hp->h_addr_list[1] != NULL) {
			int oerrno = errno;

			(void)fprintf(stderr, _("connect to address %s: "),
			    inet_ntoa(sin.sin_addr));
			__set_errno (oerrno);
			perror(0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], &sin.sin_addr,
			      MIN (sizeof (sin.sin_addr), hp->h_length));
			(void)fprintf(stderr, _("Trying %s...\n"),
			    inet_ntoa(sin.sin_addr));
			continue;
		}
		(void)fprintf(stderr, "%s: %m\n", hp->h_name);
		sigsetmask(oldmask);
		return -1;
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = rresvport(&lport), s3;
		size_t len = sizeof(from);

		if (s2 < 0)
			goto bad;
		listen(s2, 1);
		(void)snprintf(num, sizeof(num), "%d", lport);
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
			(void)fprintf(stderr,
			    _("rcmd: write (setting up stderr): %m\n"));
			(void)close(s2);
			goto bad;
		}
		FD_ZERO(&reads);
		FD_SET(s, &reads);
		FD_SET(s2, &reads);
		__set_errno (0);
		if (select(1 + (s > s2 ? s : s2), &reads, 0, 0, 0) < 1 ||
		    !FD_ISSET(s2, &reads)) {
			if (errno != 0)
				(void)fprintf(stderr,
				  _("rcmd: select (setting up stderr): %m\n"));
			else
				(void)fprintf(stderr,
			     _("select: protocol failure in circuit setup\n"));
			(void)close(s2);
			goto bad;
		}
		s3 = accept(s2, (struct sockaddr *)&from, &len);
		(void)close(s2);
		if (s3 < 0) {
			(void)fprintf(stderr,
			    "rcmd: accept: %m\n");
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED ||
		    from.sin_port < IPPORT_RESERVED / 2) {
			(void)fprintf(stderr,
			    _("socket: protocol failure in circuit setup\n"));
			goto bad2;
		}
	}
	(void)write(s, locuser, strlen(locuser)+1);
	(void)write(s, remuser, strlen(remuser)+1);
	(void)write(s, cmd, strlen(cmd)+1);
	if (read(s, &c, 1) != 1) {
		(void)fprintf(stderr,
		    "rcmd: %s: %m\n", *ahost);
		goto bad2;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void)write(STDERR_FILENO, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
	sigsetmask(oldmask);
	return s;
bad2:
	if (lport)
		(void)close(*fd2p);
bad:
	(void)close(s);
	sigsetmask(oldmask);
	return -1;
}

int
rresvport(alport)
	int *alport;
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return -1;
	for (;;) {
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
			return s;
		if (errno != EADDRINUSE) {
			(void)close(s);
			return -1;
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			(void)close(s);
			__set_errno (EAGAIN);		/* close */
			return -1;
		}
	}
}

int	__check_rhosts_file = 1;
char	*__rcmd_errstr;

int
ruserok(rhost, superuser, ruser, luser)
	const char *rhost, *ruser, *luser;
	int superuser;
{
	struct hostent hostbuf, *hp;
	size_t buflen;
	char *buffer;
	u_int32_t addr;
	char **ap;
	int herr;

	buflen = 1024;
	buffer = __alloca (buflen);

	while (__gethostbyname_r (rhost, &hostbuf, buffer, buflen, &hp, &herr)
	       < 0)
	  if (herr != NETDB_INTERNAL || errno != ERANGE)
	    return -1;
	  else
	    {
	      /* Enlarge the buffer.  */
	      buflen *= 2;
	      buffer = __alloca (buflen);
	    }

	for (ap = hp->h_addr_list; *ap; ++ap) {
		bcopy(*ap, &addr, sizeof(addr));
		if (iruserok(addr, superuser, ruser, luser) == 0)
			return 0;
	}
	return -1;
}

/* Extremely paranoid file open function. */
static FILE *
iruserfopen (char *file, uid_t okuser)
{
  struct stat st;
  char *cp = NULL;
  FILE *res = NULL;

  /* If not a regular file, if owned by someone other than user or
     root, if writeable by anyone but the owner, or if hardlinked
     anywhere, quit.  */
  cp = NULL;
  if (__lxstat (_STAT_VER, file, &st))
    cp = _("lstat failed");
  else if (!S_ISREG (st.st_mode))
    cp = _("not regular file");
  else
    {
      res = fopen (file, "r");
      if (!res)
	cp = _("cannot open");
      else if (__fxstat (_STAT_VER, fileno (res), &st) < 0)
	cp = _("fstat failed");
      else if (st.st_uid && st.st_uid != okuser)
	cp = _("bad owner");
      else if (st.st_mode & (S_IWGRP|S_IWOTH))
	cp = _("writeable by other than owner");
      else if (st.st_nlink > 1)
	cp = _("hard linked somewhere");
    }

  /* If there were any problems, quit.  */
  if (cp != NULL)
    {
      __rcmd_errstr = cp;
      if (res)
	fclose (res);
      return NULL;
    }

  return res;
}

/*
 * New .rhosts strategy: We are passed an ip address. We spin through
 * hosts.equiv and .rhosts looking for a match. When the .rhosts only
 * has ip addresses, we don't have to trust a nameserver.  When it
 * contains hostnames, we spin through the list of addresses the nameserver
 * gives us and look for a match.
 *
 * Returns 0 if ok, -1 if not ok.
 */
int
iruserok (raddr, superuser, ruser, luser)
     u_int32_t raddr;
     int superuser;
     const char *ruser, *luser;
{
  FILE *hostf = NULL;
  int isbad;

  if (!superuser)
    hostf = iruserfopen (_PATH_HEQUIV, 0);

  if (hostf)
    {
      isbad = __ivaliduser (hostf, raddr, luser, ruser);
      fclose (hostf);

      if (!isbad)
	return 0;
    }

  if (__check_rhosts_file || superuser)
    {
      char *pbuf;
      struct passwd pwdbuf, *pwd;
      size_t dirlen;
      size_t buflen = __sysconf (_SC_GETPW_R_SIZE_MAX);
      char *buffer = __alloca (buflen);
      uid_t uid;

      if (__getpwnam_r (luser, &pwdbuf, buffer, buflen, &pwd))
	return -1;

      dirlen = strlen (pwd->pw_dir);
      pbuf = alloca (dirlen + sizeof "/.rhosts");
      __mempcpy (__mempcpy (pbuf, pwd->pw_dir, dirlen),
		 "/.rhosts", sizeof "/.rhosts");

       /* Change effective uid while reading .rhosts.  If root and
	  reading an NFS mounted file system, can't read files that
	  are protected read/write owner only.  */
       uid = geteuid ();
       seteuid (pwd->pw_uid);
       hostf = iruserfopen (pbuf, pwd->pw_uid);

       if (hostf != NULL)
	 {
           isbad = __ivaliduser (hostf, raddr, luser, ruser);
           fclose (hostf);
	 }

       seteuid (uid);
       return isbad;
    }
  return -1;
}

/*
 * XXX
 * Don't make static, used by lpd(8).
 *
 * Returns 0 if ok, -1 if not ok.
 */
int
__ivaliduser(hostf, raddr, luser, ruser)
	FILE *hostf;
	u_int32_t raddr;
	const char *luser, *ruser;
{
	register char *user, *p;
	int ch;
	char *buf = NULL;
	size_t bufsize = 0;
	ssize_t nread;

	while ((nread = __getline (&buf, &bufsize, hostf)) > 0) {
		buf[bufsize - 1] = '\0'; /* Make sure it's terminated.  */
		p = buf;
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') {
			*p = isupper(*p) ? tolower(*p) : *p;
			p++;
		}
		if (*p == ' ' || *p == '\t') {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
			user = p;
			while (*p != '\n' && *p != ' ' &&
			    *p != '\t' && *p != '\0')
				p++;
		} else
			user = p;
		*p = '\0';
		if (__icheckhost(raddr, buf) &&
		    strcmp(ruser, *user ? user : luser) == 0) {
			free (buf);
			return 0;
		}
	}
	free (buf);
	return -1;
}

/*
 * Returns "true" if match, 0 if no match.
 */
static int
internal_function
__icheckhost(raddr, lhost)
	u_int32_t raddr;
	register char *lhost;
{
	struct hostent hostbuf, *hp;
	size_t buflen;
	char *buffer;
	register u_int32_t laddr;
	register char **pp;
	int herr;

	/* Try for raw ip address first. */
	if (isdigit(*lhost) && (int32_t)(laddr = inet_addr(lhost)) != -1)
		return raddr == laddr;

	/* Better be a hostname. */
	buflen = 1024;
	buffer = __alloca (buflen);
	while (__gethostbyname_r (lhost, &hostbuf, buffer, buflen, &hp, &herr)
	       < 0)
	  if (herr != NETDB_INTERNAL || errno != ERANGE)
	    return 0;
	  else
	    {
	      /* Enlarge the buffer.  */
	      buflen *= 2;
	      buffer = __alloca (buflen);
	    }

	/* Spin through ip addresses. */
	for (pp = hp->h_addr_list; *pp; ++pp)
		if (!bcmp(&raddr, *pp, sizeof(u_int32_t)))
			return 1;

	/* No match. */
	return 0;
}
