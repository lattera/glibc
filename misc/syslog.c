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
static char sccsid[] = "@(#)syslog.c	8.4 (Berkeley) 3/18/94";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/uio.h>
#include <netdb.h>

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

static int	LogFile = -1;		/* fd for log */
static int	connected;		/* have done connect */
static int	LogStat = 0;		/* status bits, set by openlog() */
static const char *LogTag = NULL;	/* string to tag the entry with */
static int	LogFacility = LOG_USER;	/* default facility code */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
extern char	*__progname;		/* Program name, from crt0. */

/*
 * syslog, vsyslog --
 *	print message on log file; output is intended for syslogd(8).
 */
void
#if __STDC__
syslog(int pri, const char *fmt, ...)
#else
syslog(pri, fmt, va_alist)
	int pri;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	vsyslog(pri, fmt, ap);
	va_end(ap);
}

void
vsyslog(pri, fmt, ap)
	int pri;
	register const char *fmt;
	va_list ap;
{
	time_t now;
	int fd;
	FILE *f;
	char *buf = 0;
	size_t bufsize = 0;
	size_t prioff, msgoff;

#define	INTERNALLOG	LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID
	/* Check for invalid bits. */
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		syslog(INTERNALLOG,
		    "syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}

	/* Check priority against setlogmask values. */
	if (!LOG_MASK(LOG_PRI(pri)) & LogMask)
		return;

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* Build the message in a memory-buffer stream.  */
	f = open_memstream (&buf, &bufsize);
	prioff = fprintf (f, "<%d>", pri);
	(void) time (&now);
#ifdef USE_IN_LIBIO
        f->_IO_write_ptr += strftime (f->_IO_write_ptr,
                                      f->_IO_write_end - f->_IO_write_ptr,
                                      "%h %e %T ", localtime (&now));
#else
	f->__bufp += strftime (f->__bufp, f->__put_limit - f->__bufp,
			       "%h %e %T ", localtime (&now));
#endif
	msgoff = ftell (f);
	if (LogTag == NULL)
	  LogTag = __progname;
	if (LogTag != NULL)
	  fputs (LogTag, f);
	if (LogStat & LOG_PID)
	  fprintf (f, "[%d]", getpid ());
	if (LogTag != NULL)
	  putc (':', f), putc (' ', f);

	/* We have the header.  Print the user's format into the buffer.  */
	vfprintf (f, fmt, ap);

	/* Close the memory stream; this will finalize the data
	   into a malloc'd buffer in BUF.  */
	fclose (f);

	/* Output to stderr if requested. */
	if (LogStat & LOG_PERROR) {
		struct iovec iov[2];
		register struct iovec *v = iov;

		v->iov_base = buf + msgoff;
		v->iov_len = bufsize - msgoff;
		++v;
		v->iov_base = (char *) "\n";
		v->iov_len = 1;
		(void)writev(STDERR_FILENO, iov, 2);
	}

	/* Get connected, output the message to the local logger. */
	if (!connected)
		openlog(LogTag, LogStat | LOG_NDELAY, 0);
	if (send(LogFile, buf, bufsize, 0) < 0)
	  {
	    /*
	     * Output the message to the console; don't worry about blocking,
	     * if console blocks everything will.  Make sure the error reported
	     * is the one from the syslogd failure.
	     */
	    if (LogStat & LOG_CONS &&
		(fd = open(_PATH_CONSOLE, O_WRONLY, 0)) >= 0)
	      {
		dprintf (fd, "%s\r\n", buf + msgoff);
		(void)close(fd);
	      }
	  }

	free (buf);
}

static struct sockaddr SyslogAddr;	/* AF_UNIX address of local logger */

void
openlog(ident, logstat, logfac)
	const char *ident;
	int logstat, logfac;
{
	if (ident != NULL)
		LogTag = ident;
	LogStat = logstat;
	if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
		LogFacility = logfac;

	if (LogFile == -1) {
		SyslogAddr.sa_family = AF_UNIX;
		(void)strncpy(SyslogAddr.sa_data, _PATH_LOG,
		    sizeof(SyslogAddr.sa_data));
		if (LogStat & LOG_NDELAY) {
			if ((LogFile = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
				return;
			(void)fcntl(LogFile, F_SETFD, 1);
		}
	}
	if (LogFile != -1 && !connected)
		if (connect(LogFile, &SyslogAddr, sizeof(SyslogAddr)) == -1) {
			(void)close(LogFile);
			LogFile = -1;
		} else
			connected = 1;
}

void
closelog()
{
	(void)close(LogFile);
	LogFile = -1;
	connected = 0;
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
