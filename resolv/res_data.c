/*
 * Copyright (c) 1995-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$BINDId: res_data.c,v 8.17 1999/10/13 17:11:31 vixie Exp $";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <ctype.h>
#include <netdb.h>
#include <resolv.h>
#ifdef BIND_UPDATE
#include <res_update.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *_res_opcodes[] = {
	"QUERY",
	"IQUERY",
	"CQUERYM",
	"CQUERYU",	/* experimental */
	"NOTIFY",	/* experimental */
	"UPDATE",
	"6",
	"7",
	"8",
	"9",
	"10",
	"11",
	"12",
	"13",
	"ZONEINIT",
	"ZONEREF",
};
libresolv_hidden_data_def (_res_opcodes)

#ifdef BIND_UPDATE
const char *_res_sectioncodes[] attribute_hidden = {
	"ZONE",
	"PREREQUISITES",
	"UPDATE",
	"ADDITIONAL",
};
#endif

#ifndef __BIND_NOSTATIC
#ifdef _LIBC
/* The definition has been moved to res_libc.c.  */
#else
#undef _res
struct __res_state _res
# if defined(__BIND_RES_TEXT)
	= { RES_TIMEOUT, }	/* Motorola, et al. */
# endif
        ;
#endif

/* Proto. */
#ifndef _LIBC
int  res_ourserver_p(const res_state, const struct sockaddr_in *);
void res_pquery(const res_state, const u_char *, int, FILE *);
#endif

#ifndef _LIBC
/* Moved to res_libc.c since res_init() should go into libc.so but the
   rest of this file not.  */
int
res_init(void) {
	extern int __res_vinit(res_state, int);

	/*
	 * These three fields used to be statically initialized.  This made
	 * it hard to use this code in a shared library.  It is necessary,
	 * now that we're doing dynamic initialization here, that we preserve
	 * the old semantics: if an application modifies one of these three
	 * fields of _res before res_init() is called, res_init() will not
	 * alter them.  Of course, if an application is setting them to
	 * _zero_ before calling res_init(), hoping to override what used
	 * to be the static default, we can't detect it and unexpected results
	 * will follow.  Zero for any of these fields would make no sense,
	 * so one can safely assume that the applications were already getting
	 * unexpected results.
	 *
	 * _res.options is tricky since some apps were known to diddle the bits
	 * before res_init() was first called. We can't replicate that semantic
	 * with dynamic initialization (they may have turned bits off that are
	 * set in RES_DEFAULT).  Our solution is to declare such applications
	 * "broken".  They could fool us by setting RES_INIT but none do (yet).
	 */
	if (!_res.retrans)
		_res.retrans = RES_TIMEOUT;
	if (!_res.retry)
		_res.retry = 4;
	if (!(_res.options & RES_INIT))
		_res.options = RES_DEFAULT;

	/*
	 * This one used to initialize implicitly to zero, so unless the app
	 * has set it to something in particular, we can randomize it now.
	 */
	if (!_res.id)
		_res.id = res_randomid();

	return (__res_vinit(&_res, 1));
}
#endif

void
p_query(const u_char *msg) {
	fp_query(msg, stdout);
}

void
fp_query(const u_char *msg, FILE *file) {
	fp_nquery(msg, PACKETSZ, file);
}
libresolv_hidden_def (fp_query)

void
fp_nquery(const u_char *msg, int len, FILE *file) {
	if (__res_maybe_init (&_res, 0) == -1)
		return;

	res_pquery(&_res, msg, len, file);
}
libresolv_hidden_def (fp_nquery)

int
res_mkquery(int op,			/* opcode of query */
	    const char *dname,		/* domain name */
	    int class, int type,	/* class and type of query */
	    const u_char *data,		/* resource record data */
	    int datalen,		/* length of data */
	    const u_char *newrr_in,	/* new rr for modify or append */
	    u_char *buf,		/* buffer to put query */
	    int buflen)			/* size of buffer */
{
	if (__res_maybe_init (&_res, 1) == -1) {
		RES_SET_H_ERRNO(&_res, NETDB_INTERNAL);
		return (-1);
	}
	return (res_nmkquery(&_res, op, dname, class, type,
			     data, datalen,
			     newrr_in, buf, buflen));
}

#ifdef BIND_UPDATE
int
res_mkupdate(ns_updrec *rrecp_in, u_char *buf, int buflen) {
	if (__res_maybe_init (&_res, 1) == -1) {
		RES_SET_H_ERRNO(&_res, NETDB_INTERNAL);
		return (-1);
	}

	return (res_nmkupdate(&_res, rrecp_in, buf, buflen));
}
#endif

int
res_query(const char *name,	/* domain name */
	  int class, int type,	/* class and type of query */
	  u_char *answer,	/* buffer to put answer */
	  int anslen)		/* size of answer buffer */
{
	if (__res_maybe_init (&_res, 1) == -1) {
		RES_SET_H_ERRNO(&_res, NETDB_INTERNAL);
		return (-1);
	}
	return (res_nquery(&_res, name, class, type, answer, anslen));
}

void
res_send_setqhook(res_send_qhook hook) {
	_res.qhook = hook;
}

void
res_send_setrhook(res_send_rhook hook) {
	_res.rhook = hook;
}

int
res_isourserver(const struct sockaddr_in *inp) {
	return (res_ourserver_p(&_res, (const struct sockaddr_in6 *) inp));
}

int
res_send(const u_char *buf, int buflen, u_char *ans, int anssiz) {
	if (__res_maybe_init (&_res, 1) == -1) {
		/* errno should have been set by res_init() in this case. */
		return (-1);
	}

	return (res_nsend(&_res, buf, buflen, ans, anssiz));
}

#ifndef _LIBC
int
res_sendsigned(const u_char *buf, int buflen, ns_tsig_key *key,
	       u_char *ans, int anssiz)
{
	if (__res_maybe_init (&_res, 1) == -1) {
		/* errno should have been set by res_init() in this case. */
		return (-1);
	}

	return (res_nsendsigned(&_res, buf, buflen, key, ans, anssiz));
}
#endif

void
res_close(void) {
#ifdef _LIBC
  	/*
	 * Some stupid programs out there call res_close() before res_init().
	 * Since _res._vcsock isn't explicitly initialized, these means that
	 * we could do a close(0), which might lead to some security problems.
	 * Therefore we check if res_init() was called before by looking at
	 * the RES_INIT bit in _res.options.  If it hasn't been set we bail out
	 * early.  */
	if ((_res.options & RES_INIT) == 0)
	  return;
#endif
	res_nclose(&_res);
}

#ifdef BIND_UPDATE
int
res_update(ns_updrec *rrecp_in) {
	if (__res_maybe_init (&_res, 1) == -1) {
		RES_SET_H_ERRNO(&_res, NETDB_INTERNAL);
		return (-1);
	}

	return (res_nupdate(&_res, rrecp_in, NULL));
}
#endif

int
res_search(const char *name,	/* domain name */
	   int class, int type,	/* class and type of query */
	   u_char *answer,	/* buffer to put answer */
	   int anslen)		/* size of answer */
{
	if (__res_maybe_init (&_res, 1) == -1) {
		RES_SET_H_ERRNO(&_res, NETDB_INTERNAL);
		return (-1);
	}

	return (res_nsearch(&_res, name, class, type, answer, anslen));
}

int
res_querydomain(const char *name,
		const char *domain,
		int class, int type,	/* class and type of query */
		u_char *answer,		/* buffer to put answer */
		int anslen)		/* size of answer */
{
	if (__res_maybe_init (&_res, 1) == -1) {
		RES_SET_H_ERRNO(&_res, NETDB_INTERNAL);
		return (-1);
	}

	return (res_nquerydomain(&_res, name, domain,
				 class, type,
				 answer, anslen));
}

const char *
hostalias(const char *name) {
	static char abuf[MAXDNAME];

	return (res_hostalias(&_res, name, abuf, sizeof abuf));
}
libresolv_hidden_def (hostalias)

#ifdef ultrix
int
local_hostname_length(const char *hostname) {
	int len_host, len_domain;

	if (!*_res.defdname)
		res_init();
	len_host = strlen(hostname);
	len_domain = strlen(_res.defdname);
	if (len_host > len_domain &&
	    !strcasecmp(hostname + len_host - len_domain, _res.defdname) &&
	    hostname[len_host - len_domain - 1] == '.')
		return (len_host - len_domain - 1);
	return (0);
}
#endif /*ultrix*/

#endif


#include <shlib-compat.h>

#if SHLIB_COMPAT(libresolv, GLIBC_2_0, GLIBC_2_2)
# undef res_mkquery
# undef res_query
# undef res_querydomain
# undef res_search
weak_alias (__res_mkquery, res_mkquery);
weak_alias (__res_query, res_query);
weak_alias (__res_querydomain, res_querydomain);
weak_alias (__res_search, res_search);
#endif
