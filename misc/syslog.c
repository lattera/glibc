/*
 * Copyright (c) 1983, 1988, 1993
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
static char sccsid[] = "@(#)syslog.c	8.4 (Berkeley) 3/18/94";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <netdb.h>

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <bits/libc-lock.h>
#include <signal.h>
#include <locale.h>

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <libio/iolibio.h>
#include <math_ldbl_opt.h>

#define ftell(s) INTUSE(_IO_ftell) (s)

static int	LogType = SOCK_DGRAM;	/* type of socket connection */
static int	LogFile = -1;		/* fd for log */
static int	connected;		/* have done connect */
static int	LogStat;		/* status bits, set by openlog() */
static const char *LogTag;		/* string to tag the entry with */
static int	LogFacility = LOG_USER;	/* default facility code */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
extern char	*__progname;		/* Program name, from crt0. */

/* Define the lock.  */
__libc_lock_define_initialized (static, syslog_lock)

static void openlog_internal(const char *, int, int) internal_function;
static void closelog_internal(void);
#ifndef NO_SIGPIPE
static void sigpipe_handler (int);
#endif

#ifndef send_flags
# define send_flags 0
#endif

struct cleanup_arg
{
  void *buf;
  struct sigaction *oldaction;
};

static void
cancel_handler (void *ptr)
{
#ifndef NO_SIGPIPE
  /* Restore the old signal handler.  */
  struct cleanup_arg *clarg = (struct cleanup_arg *) ptr;

  if (clarg != NULL && clarg->oldaction != NULL)
    __sigaction (SIGPIPE, clarg->oldaction, NULL);
#endif

  /* Free the lock.  */
  __libc_lock_unlock (syslog_lock);
}


/*
 * syslog, vsyslog --
 *	print message on log file; output is intended for syslogd(8).
 */
void
__syslog(int pri, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__vsyslog_chk(pri, -1, fmt, ap);
	va_end(ap);
}
ldbl_hidden_def (__syslog, syslog)
ldbl_strong_alias (__syslog, syslog)

void
__syslog_chk(int pri, int flag, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__vsyslog_chk(pri, flag, fmt, ap);
	va_end(ap);
}

void
__vsyslog_chk(int pri, int flag, const char *fmt, va_list ap)
{
	struct tm now_tm;
	time_t now;
	int fd;
	FILE *f;
	char *buf = 0;
	size_t bufsize = 0;
	size_t msgoff;
#ifndef NO_SIGPIPE
 	struct sigaction action, oldaction;
 	int sigpipe;
#endif
	int saved_errno = errno;
	char failbuf[3 * sizeof (pid_t) + sizeof "out of memory []"];

#define	INTERNALLOG	LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID
	/* Check for invalid bits. */
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		syslog(INTERNALLOG,
		    "syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}

	/* Check priority against setlogmask values. */
	if ((LOG_MASK (LOG_PRI (pri)) & LogMask) == 0)
		return;

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* Build the message in a memory-buffer stream.  */
	f = open_memstream (&buf, &bufsize);
	if (f == NULL)
	  {
	    /* We cannot get a stream.  There is not much we can do but
	       emitting an error messages.  */
	    char numbuf[3 * sizeof (pid_t)];
	    char *nump;
	    char *endp = __stpcpy (failbuf, "out of memory [");
	    pid_t pid = __getpid ();

	    nump = numbuf + sizeof (numbuf);
	    /* The PID can never be zero.  */
	    do
	      *--nump = '0' + pid % 10;
	    while ((pid /= 10) != 0);

	    endp = __mempcpy (endp, nump, (numbuf + sizeof (numbuf)) - nump);
	    *endp++ = ']';
	    *endp = '\0';
	    buf = failbuf;
	    bufsize = endp - failbuf;
	    msgoff = 0;
	  }
	else
	  {
	    __fsetlocking (f, FSETLOCKING_BYCALLER);
	    fprintf (f, "<%d>", pri);
	    (void) time (&now);
	    f->_IO_write_ptr += __strftime_l (f->_IO_write_ptr,
					      f->_IO_write_end
					      - f->_IO_write_ptr,
					      "%h %e %T ",
					      __localtime_r (&now, &now_tm),
					      _nl_C_locobj_ptr);
	    msgoff = ftell (f);
	    if (LogTag == NULL)
	      LogTag = __progname;
	    if (LogTag != NULL)
	      fputs_unlocked (LogTag, f);
	    if (LogStat & LOG_PID)
	      fprintf (f, "[%d]", (int) __getpid ());
	    if (LogTag != NULL)
	      {
		putc_unlocked (':', f);
		putc_unlocked (' ', f);
	      }

	    /* Restore errno for %m format.  */
	    __set_errno (saved_errno);

	    /* We have the header.  Print the user's format into the
               buffer.  */
	    if (flag == -1)
	      vfprintf (f, fmt, ap);
	    else
	      __vfprintf_chk (f, flag, fmt, ap);

	    /* Close the memory stream; this will finalize the data
	       into a malloc'd buffer in BUF.  */
	    fclose (f);
	  }

	/* Output to stderr if requested. */
	if (LogStat & LOG_PERROR) {
		struct iovec iov[2];
		register struct iovec *v = iov;

		v->iov_base = buf + msgoff;
		v->iov_len = bufsize - msgoff;
		/* Append a newline if necessary.  */
		if (buf[bufsize - 1] != '\n')
		  {
		    ++v;
		    v->iov_base = (char *) "\n";
		    v->iov_len = 1;
		  }

		__libc_cleanup_push (free, buf == failbuf ? NULL : buf);

		/* writev is a cancellation point.  */
		(void)__writev(STDERR_FILENO, iov, v - iov + 1);

		__libc_cleanup_pop (0);
	}

	/* Prepare for multiple users.  We have to take care: open and
	   write are cancellation points.  */
	struct cleanup_arg clarg;
	clarg.buf = buf;
	clarg.oldaction = NULL;
	__libc_cleanup_push (cancel_handler, &clarg);
	__libc_lock_lock (syslog_lock);

#ifndef NO_SIGPIPE
	/* Prepare for a broken connection.  */
 	memset (&action, 0, sizeof (action));
 	action.sa_handler = sigpipe_handler;
 	sigemptyset (&action.sa_mask);
 	sigpipe = __sigaction (SIGPIPE, &action, &oldaction);
	if (sigpipe == 0)
	  clarg.oldaction = &oldaction;
#endif

	/* Get connected, output the message to the local logger. */
	if (!connected)
		openlog_internal(LogTag, LogStat | LOG_NDELAY, 0);

	/* If we have a SOCK_STREAM connection, also send ASCII NUL as
	   a record terminator.  */
	if (LogType == SOCK_STREAM)
	  ++bufsize;

	if (!connected || __send(LogFile, buf, bufsize, send_flags) < 0)
	  {
	    if (connected)
	      {
		/* Try to reopen the syslog connection.  Maybe it went
		   down.  */
		closelog_internal ();
		openlog_internal(LogTag, LogStat | LOG_NDELAY, 0);
	      }

	    if (!connected || __send(LogFile, buf, bufsize, send_flags) < 0)
	      {
		closelog_internal ();	/* attempt re-open next time */
		/*
		 * Output the message to the console; don't worry
		 * about blocking, if console blocks everything will.
		 * Make sure the error reported is the one from the
		 * syslogd failure.
		 */
		if (LogStat & LOG_CONS &&
		    (fd = __open(_PATH_CONSOLE, O_WRONLY|O_NOCTTY, 0)) >= 0)
		  {
		    dprintf (fd, "%s\r\n", buf + msgoff);
		    (void)__close(fd);
		  }
	      }
	  }

#ifndef NO_SIGPIPE
	if (sigpipe == 0)
		__sigaction (SIGPIPE, &oldaction, (struct sigaction *) NULL);
#endif

	/* End of critical section.  */
	__libc_cleanup_pop (0);
	__libc_lock_unlock (syslog_lock);

	if (buf != failbuf)
		free (buf);
}
libc_hidden_def (__vsyslog_chk)

void
__vsyslog(int pri, const char *fmt, va_list ap)
{
  __vsyslog_chk (pri, -1, fmt, ap);
}
ldbl_hidden_def (__vsyslog, vsyslog)
ldbl_strong_alias (__vsyslog, vsyslog)

static struct sockaddr_un SyslogAddr;	/* AF_UNIX address of local logger */


static void
internal_function
openlog_internal(const char *ident, int logstat, int logfac)
{
	if (ident != NULL)
		LogTag = ident;
	LogStat = logstat;
	if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
		LogFacility = logfac;

	int retry = 0;
	while (retry < 2) {
		if (LogFile == -1) {
			SyslogAddr.sun_family = AF_UNIX;
			(void)strncpy(SyslogAddr.sun_path, _PATH_LOG,
				      sizeof(SyslogAddr.sun_path));
			if (LogStat & LOG_NDELAY) {
#ifdef SOCK_CLOEXEC
# ifndef __ASSUME_SOCK_CLOEXEC
				if (__have_sock_cloexec >= 0) {
# endif
					LogFile = __socket(AF_UNIX,
							   LogType
							   | SOCK_CLOEXEC, 0);
# ifndef __ASSUME_SOCK_CLOEXEC
					if (__have_sock_cloexec == 0)
						__have_sock_cloexec
						  = ((LogFile != -1
						      || errno != EINVAL)
						     ? 1 : -1);
				}
# endif
#endif
#ifndef __ASSUME_SOCK_CLOEXEC
# ifdef SOCK_CLOEXEC
				if (__have_sock_cloexec < 0)
# endif
				  LogFile = __socket(AF_UNIX, LogType, 0);
#endif
				if (LogFile == -1)
					return;
#ifndef __ASSUME_SOCK_CLOEXEC
# ifdef SOCK_CLOEXEC
				if (__have_sock_cloexec < 0)
# endif
					__fcntl(LogFile, F_SETFD, FD_CLOEXEC);
#endif
			}
		}
		if (LogFile != -1 && !connected)
		{
			int old_errno = errno;
			if (__connect(LogFile, &SyslogAddr, sizeof(SyslogAddr))
			    == -1)
			{
				int saved_errno = errno;
				int fd = LogFile;
				LogFile = -1;
				(void)__close(fd);
				__set_errno (old_errno);
				if (saved_errno == EPROTOTYPE)
				{
					/* retry with the other type: */
					LogType = (LogType == SOCK_DGRAM
						   ? SOCK_STREAM : SOCK_DGRAM);
					++retry;
					continue;
				}
			} else
				connected = 1;
		}
		break;
	}
}

void
openlog (const char *ident, int logstat, int logfac)
{
  /* Protect against multiple users and cancellation.  */
  __libc_cleanup_push (cancel_handler, NULL);
  __libc_lock_lock (syslog_lock);

  openlog_internal (ident, logstat, logfac);

  __libc_cleanup_pop (1);
}

#ifndef NO_SIGPIPE
static void
sigpipe_handler (int signo)
{
  closelog_internal ();
}
#endif

static void
closelog_internal()
{
  if (!connected)
    return;

  __close (LogFile);
  LogFile = -1;
  connected = 0;
}

void
closelog ()
{
  /* Protect against multiple users and cancellation.  */
  __libc_cleanup_push (cancel_handler, NULL);
  __libc_lock_lock (syslog_lock);

  closelog_internal ();
  LogTag = NULL;
  LogType = SOCK_DGRAM; /* this is the default */

  /* Free the lock.  */
  __libc_cleanup_pop (1);
}

/* setlogmask -- set the log mask level */
int
setlogmask(pmask)
	int pmask;
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}
