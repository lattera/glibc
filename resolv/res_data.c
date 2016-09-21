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


void
res_close(void) {
	/*
	 * Some stupid programs out there call res_close() before res_init().
	 * Since _res._vcsock isn't explicitly initialized, these means that
	 * we could do a close(0), which might lead to some security problems.
	 * Therefore we check if res_init() was called before by looking at
	 * the RES_INIT bit in _res.options.  If it hasn't been set we bail out
	 * early.  */
	if ((_res.options & RES_INIT) == 0)
	  return;
	/* We don't free the name server addresses because we never
	   did it and it would be done implicitly on shutdown.  */
	__res_iclose(&_res, false);
}

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
