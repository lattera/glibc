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
#include <sys/poll.h>
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


int __ivaliduser __P ((FILE *, u_int32_t, const char *, const char *));
static int __ivaliduser2 __P ((FILE *, u_int32_t, const char *, const char *,
			       const char *));


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
	struct pollfd pfd[2];
	int32_t oldmask;
	pid_t pid;
	int s, lport, timo;
	char c;
	int herr;

	pid = __getpid();

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

	pfd[0].events = POLLIN;
	pfd[1].events = POLLIN;

	*ahost = hp->h_name;
	oldmask = __sigblock(sigmask(SIGURG));
	for (timo = 1, lport = IPPORT_RESERVED - 1;;) {
		s = rresvport(&lport);
		if (s < 0) {
			if (errno == EAGAIN)
				(void)fprintf(stderr,
				    _("rcmd: socket: All ports in use\n"));
			else
				(void)fprintf(stderr, "rcmd: socket: %m\n");
			__sigsetmask(oldmask);
			return -1;
		}
		__fcntl(s, F_SETOWN, pid);
		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], &sin.sin_addr,
		      MIN (sizeof (sin.sin_addr), hp->h_length));
		sin.sin_port = rport;
		if (__connect(s, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
			break;
		(void)__close(s);
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
			(void)__sleep(timo);
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
		__sigsetmask(oldmask);
		return -1;
	}
	lport--;
	if (fd2p == 0) {
		__write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = rresvport(&lport), s3;
		size_t len = sizeof(from);

		if (s2 < 0)
			goto bad;
		listen(s2, 1);
		(void)__snprintf(num, sizeof(num), "%d", lport);
		if (__write(s, num, strlen(num)+1) != strlen(num)+1) {
			(void)fprintf(stderr,
			    _("rcmd: write (setting up stderr): %m\n"));
			(void)__close(s2);
			goto bad;
		}
		pfd[0].fd = s;
		pfd[1].fd = s2;
		__set_errno (0);
		if (__poll (pfd, 2, -1) < 1 || (pfd[1].revents & POLLIN) == 0){
			if (errno != 0)
				(void)fprintf(stderr,
				  _("rcmd: poll (setting up stderr): %m\n"));
			else
				(void)fprintf(stderr,
			     _("poll: protocol failure in circuit setup\n"));
			(void)__close(s2);
			goto bad;
		}
		s3 = accept(s2, (struct sockaddr *)&from, &len);
		(void)__close(s2);
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
	(void)__write(s, locuser, strlen(locuser)+1);
	(void)__write(s, remuser, strlen(remuser)+1);
	(void)__write(s, cmd, strlen(cmd)+1);
	if (__read(s, &c, 1) != 1) {
		(void)fprintf(stderr,
		    "rcmd: %s: %m\n", *ahost);
		goto bad2;
	}
	if (c != 0) {
		while (__read(s, &c, 1) == 1) {
			(void)__write(STDERR_FILENO, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
	__sigsetmask(oldmask);
	return s;
bad2:
	if (lport)
		(void)__close(*fd2p);
bad:
	(void)__close(s);
	__sigsetmask(oldmask);
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
	s = __socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return -1;
	for (;;) {
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
			return s;
		if (errno != EADDRINUSE) {
			(void)__close(s);
			return -1;
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			(void)__close(s);
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
		if (iruserok(addr, superuser, ruser, luser, rhost) == 0)
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
static int
iruserok2 (raddr, superuser, ruser, luser, rhost)
     u_int32_t raddr;
     int superuser;
     const char *ruser, *luser, *rhost;
{
  FILE *hostf = NULL;
  int isbad;

  if (!superuser)
    hostf = iruserfopen (_PATH_HEQUIV, 0);

  if (hostf)
    {
      isbad = __ivaliduser2 (hostf, raddr, luser, ruser, rhost);
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
       uid = __geteuid ();
       seteuid (pwd->pw_uid);
       hostf = iruserfopen (pbuf, pwd->pw_uid);

       if (hostf != NULL)
	 {
           isbad = __ivaliduser2 (hostf, raddr, luser, ruser, rhost);
           fclose (hostf);
	 }

       seteuid (uid);
       return isbad;
    }
  return -1;
}

/* This is the exported version.  */
int
iruserok (raddr, superuser, ruser, luser)
     u_int32_t raddr;
     int superuser;
     const char *ruser, *luser;
{
  return iruserok2 (raddr, superuser, ruser, luser, "-");
}

/*
 * XXX
 * Don't make static, used by lpd(8).
 *
 * This function is not used anymore. It is only present because lpd(8)
 * calls it (!?!). We simply call __invaliduser2() with an illegal rhost
 * argument. This means that netgroups won't work in .rhost/hosts.equiv
 * files. If you want lpd to work with netgroups, fix lpd to use ruserok()
 * or PAM.
 * Returns 0 if ok, -1 if not ok.
 */
int
__ivaliduser(hostf, raddr, luser, ruser)
	FILE *hostf;
	u_int32_t raddr;
	const char *luser, *ruser;
{
	return __ivaliduser2(hostf, raddr, luser, ruser, "-");
}


/* Returns 1 on positive match, 0 on no match, -1 on negative match.  */
static int
internal_function
__icheckhost (raddr, lhost, rhost)
	u_int32_t raddr;
	char *lhost;
	const char *rhost;
{
	struct hostent hostbuf, *hp;
	size_t buflen;
	char *buffer;
	int herr;
	int save_errno;
	u_int32_t laddr;
	int negate=1;    /* Multiply return with this to get -1 instead of 1 */
	char **pp, *user;

	/* Check nis netgroup.  */
	if (strncmp ("+@", lhost, 2) == 0)
		return innetgr (&lhost[2], rhost, NULL, NULL);

	if (strncmp ("-@", lhost, 2) == 0)
		return -innetgr (&lhost[2], rhost, NULL, NULL);

	/* -host */
	if (strncmp ("-", lhost,1) == 0) {
		negate = -1;
		lhost++;
	} else if (strcmp ("+",lhost) == 0) {
		return 1;                    /* asking for trouble, but ok.. */
	}

	/* Try for raw ip address first. */
	if (isdigit (*lhost) && (long) (laddr = inet_addr (lhost)) != -1)
		return negate * (! (raddr ^ laddr));

	/* Better be a hostname. */
	buflen = 1024;
	buffer = __alloca (buflen);
	save_errno = errno;
	while (__gethostbyname_r (lhost, &hostbuf, buffer, buflen, &hp, &herr)
	       < 0)
		if (herr != NETDB_INTERNAL || errno != ERANGE)
			return (0);
		else {
			/* Enlarge the buffer.  */
			buflen *= 2;
			buffer = __alloca (buflen);
			__set_errno (0);
		}
	__set_errno (save_errno);
	if (hp == NULL)
		return 0;

	/* Spin through ip addresses. */
	for (pp = hp->h_addr_list; *pp; ++pp)
		if (!memcmp (&raddr, *pp, sizeof (u_int32_t)))
			return negate;

	/* No match. */
	return (0);
}

/* Returns 1 on positive match, 0 on no match, -1 on negative match.  */
static int
internal_function
__icheckuser (luser, ruser)
	const char *luser, *ruser;
{
    /*
      luser is user entry from .rhosts/hosts.equiv file
      ruser is user id on remote host
      */
    char *user;

    /* [-+]@netgroup */
    if (strncmp ("+@", luser, 2) == 0)
	return innetgr (&luser[2], NULL, ruser, NULL);

    if (strncmp ("-@", luser,2) == 0)
	return -innetgr (&luser[2], NULL, ruser, NULL);

    /* -user */
    if (strncmp ("-", luser, 1) == 0)
	return -(strcmp (&luser[1], ruser) == 0);

    /* + */
    if (strcmp ("+", luser) == 0)
	return 1;

    /* simple string match */
    return strcmp (ruser, luser) == 0;
}

/*
 * Returns 1 for blank lines (or only comment lines) and 0 otherwise
 */
static int
__isempty(p)
	char *p;
{
    while (*p && isspace (*p)) {
	++p;
    }

    return (*p == '\0' || *p == '#') ? 1 : 0 ;
}

/*
 * Returns 0 if positive match, -1 if _not_ ok.
 */
static int
__ivaliduser2(hostf, raddr, luser, ruser, rhost)
	FILE *hostf;
	u_int32_t raddr;
	const char *luser, *ruser, *rhost;
{
    register const char *user;
    register char *p;
    int hcheck, ucheck;
    char *buf = NULL;
    size_t bufsize = 0;

    while (__getline (&buf, &bufsize, hostf) > 0) {
	buf[bufsize - 1] = '\0'; /* Make sure it's terminated.  */
        p = buf;

	/* Skip empty or comment lines */
	if (__isempty (p)) {
	    continue;
	}

	/* Skip lines that are too long. */
	if (strchr (p, '\n') == NULL) {
	    int ch = getc (hostf);

	    while (ch != '\n' && ch != EOF)
		ch = getc (hostf);
	    continue;
	}

	for (;*p && !isspace(*p); ++p) {
	    *p = tolower (*p);
	}

	/* Next we want to find the permitted name for the remote user.  */
	if (*p == ' ' || *p == '\t') {
	    /* <nul> terminate hostname and skip spaces */
	    for (*p++='\0'; *p && isspace (*p); ++p);

	    user = p;                   /* this is the user's name */
	    while (*p && !isspace (*p))
		++p;                    /* find end of user's name */
	} else
	    user = p;

	*p = '\0';              /* <nul> terminate username (+host?) */

	/* buf -> host(?) ; user -> username(?) */

	/* First check host part */
	hcheck = __icheckhost (raddr, buf, rhost);

	if (hcheck < 0)
	    return -1;

	if (hcheck) {
	    /* Then check user part */
	    if (! (*user))
		user = luser;

	    ucheck = __icheckuser (user, ruser);

	    /* Positive 'host user' match? */
	    if (ucheck > 0)
		return 0;

	    /* Negative 'host -user' match? */
	    if (ucheck < 0)
		return -1;

	    /* Neither, go on looking for match */
	}
    }

    return -1;
}
