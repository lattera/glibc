/*
 * Copyright (c) 1985, 1989, 1993
 *    The Regents of the University of California.  All rights reserved.
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

/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
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
static const char sccsid[] = "@(#)res_send.c	8.1 (Berkeley) 6/4/93";
static const char rcsid[] = "$BINDId: res_send.c,v 8.38 2000/03/30 20:16:51 vixie Exp $";
#endif /* LIBC_SCCS and not lint */

/*
 * Send query to name server and wait for reply.
 */

#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/poll.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <kernel-features.h>

#if PACKETSZ > 65536
#define MAXPACKET       PACKETSZ
#else
#define MAXPACKET       65536
#endif


#ifndef __ASSUME_SOCK_CLOEXEC
static int __have_o_nonblock;
#else
# define __have_o_nonblock 0
#endif


/* From ev_streams.c.  */

static inline void
__attribute ((always_inline))
evConsIovec(void *buf, size_t cnt, struct iovec *vec) {
	memset(vec, 0xf5, sizeof (*vec));
	vec->iov_base = buf;
	vec->iov_len = cnt;
}

/* From ev_timers.c.  */

#define BILLION 1000000000

static inline void
evConsTime(struct timespec *res, time_t sec, long nsec) {
	res->tv_sec = sec;
	res->tv_nsec = nsec;
}

static inline void
evAddTime(struct timespec *res, const struct timespec *addend1,
	  const struct timespec *addend2) {
	res->tv_sec = addend1->tv_sec + addend2->tv_sec;
	res->tv_nsec = addend1->tv_nsec + addend2->tv_nsec;
	if (res->tv_nsec >= BILLION) {
		res->tv_sec++;
		res->tv_nsec -= BILLION;
	}
}

static inline void
evSubTime(struct timespec *res, const struct timespec *minuend,
	  const struct timespec *subtrahend) {
       res->tv_sec = minuend->tv_sec - subtrahend->tv_sec;
	if (minuend->tv_nsec >= subtrahend->tv_nsec)
		res->tv_nsec = minuend->tv_nsec - subtrahend->tv_nsec;
	else {
		res->tv_nsec = (BILLION
				- subtrahend->tv_nsec + minuend->tv_nsec);
		res->tv_sec--;
	}
}

static inline int
evCmpTime(struct timespec a, struct timespec b) {
	long x = a.tv_sec - b.tv_sec;

	if (x == 0L)
		x = a.tv_nsec - b.tv_nsec;
	return (x < 0L ? (-1) : x > 0L ? (1) : (0));
}

static inline void
evNowTime(struct timespec *res) {
	struct timeval now;

	if (gettimeofday(&now, NULL) < 0)
		evConsTime(res, 0, 0);
	else
		TIMEVAL_TO_TIMESPEC (&now, res);
}


/* Options.  Leave them on. */
/* #undef DEBUG */
#include "res_debug.h"

#define EXT(res) ((res)->_u._ext)

/* Forward. */

static int		send_vc(res_state, const u_char *, int,
				const u_char *, int,
				u_char **, int *, int *, int, u_char **,
				u_char **, int *, int *);
static int		send_dg(res_state, const u_char *, int,
				const u_char *, int,
				u_char **, int *, int *, int,
				int *, int *, u_char **,
				u_char **, int *, int *);
#ifdef DEBUG
static void		Aerror(const res_state, FILE *, const char *, int,
			       const struct sockaddr *);
static void		Perror(const res_state, FILE *, const char *, int);
#endif
static int		sock_eq(struct sockaddr_in6 *, struct sockaddr_in6 *);

/* Public. */

/* int
 * res_isourserver(ina)
 *	looks up "ina" in _res.ns_addr_list[]
 * returns:
 *	0  : not found
 *	>0 : found
 * author:
 *	paul vixie, 29may94
 */
int
res_ourserver_p(const res_state statp, const struct sockaddr_in6 *inp)
{
	int ns;

	if (inp->sin6_family == AF_INET) {
	    struct sockaddr_in *in4p = (struct sockaddr_in *) inp;
	    in_port_t port = in4p->sin_port;
	    in_addr_t addr = in4p->sin_addr.s_addr;

	    for (ns = 0;  ns < MAXNS;  ns++) {
		const struct sockaddr_in *srv =
		    (struct sockaddr_in *)EXT(statp).nsaddrs[ns];

		if ((srv != NULL) && (srv->sin_family == AF_INET) &&
		    (srv->sin_port == port) &&
		    (srv->sin_addr.s_addr == INADDR_ANY ||
		     srv->sin_addr.s_addr == addr))
		    return (1);
	    }
	} else if (inp->sin6_family == AF_INET6) {
	    for (ns = 0;  ns < MAXNS;  ns++) {
		const struct sockaddr_in6 *srv = EXT(statp).nsaddrs[ns];
		if ((srv != NULL) && (srv->sin6_family == AF_INET6) &&
		    (srv->sin6_port == inp->sin6_port) &&
		    !(memcmp(&srv->sin6_addr, &in6addr_any,
			     sizeof (struct in6_addr)) &&
		      memcmp(&srv->sin6_addr, &inp->sin6_addr,
			     sizeof (struct in6_addr))))
		    return (1);
	    }
	}
	return (0);
}

/* int
 * res_nameinquery(name, type, class, buf, eom)
 *	look for (name,type,class) in the query section of packet (buf,eom)
 * requires:
 *	buf + HFIXEDSZ <= eom
 * returns:
 *	-1 : format error
 *	0  : not found
 *	>0 : found
 * author:
 *	paul vixie, 29may94
 */
int
res_nameinquery(const char *name, int type, int class,
		const u_char *buf, const u_char *eom)
{
	const u_char *cp = buf + HFIXEDSZ;
	int qdcount = ntohs(((HEADER*)buf)->qdcount);

	while (qdcount-- > 0) {
		char tname[MAXDNAME+1];
		int n, ttype, tclass;

		n = dn_expand(buf, eom, cp, tname, sizeof tname);
		if (n < 0)
			return (-1);
		cp += n;
		if (cp + 2 * INT16SZ > eom)
			return (-1);
		NS_GET16(ttype, cp);
		NS_GET16(tclass, cp);
		if (ttype == type && tclass == class &&
		    ns_samename(tname, name) == 1)
			return (1);
	}
	return (0);
}
libresolv_hidden_def (res_nameinquery)

/* int
 * res_queriesmatch(buf1, eom1, buf2, eom2)
 *	is there a 1:1 mapping of (name,type,class)
 *	in (buf1,eom1) and (buf2,eom2)?
 * returns:
 *	-1 : format error
 *	0  : not a 1:1 mapping
 *	>0 : is a 1:1 mapping
 * author:
 *	paul vixie, 29may94
 */
int
res_queriesmatch(const u_char *buf1, const u_char *eom1,
		 const u_char *buf2, const u_char *eom2)
{
	if (buf1 + HFIXEDSZ > eom1 || buf2 + HFIXEDSZ > eom2)
		return (-1);

	/*
	 * Only header section present in replies to
	 * dynamic update packets.
	 */
	if ((((HEADER *)buf1)->opcode == ns_o_update) &&
	    (((HEADER *)buf2)->opcode == ns_o_update))
		return (1);

	/* Note that we initially do not convert QDCOUNT to the host byte
	   order.  We can compare it with the second buffer's QDCOUNT
	   value without doing this.  */
	int qdcount = ((HEADER*)buf1)->qdcount;
	if (qdcount != ((HEADER*)buf2)->qdcount)
		return (0);

	qdcount = htons (qdcount);
	const u_char *cp = buf1 + HFIXEDSZ;

	while (qdcount-- > 0) {
		char tname[MAXDNAME+1];
		int n, ttype, tclass;

		n = dn_expand(buf1, eom1, cp, tname, sizeof tname);
		if (n < 0)
			return (-1);
		cp += n;
		if (cp + 2 * INT16SZ > eom1)
			return (-1);
		NS_GET16(ttype, cp);
		NS_GET16(tclass, cp);
		if (!res_nameinquery(tname, ttype, tclass, buf2, eom2))
			return (0);
	}
	return (1);
}
libresolv_hidden_def (res_queriesmatch)

int
__libc_res_nsend(res_state statp, const u_char *buf, int buflen,
		 const u_char *buf2, int buflen2,
		 u_char *ans, int anssiz, u_char **ansp, u_char **ansp2,
		 int *nansp2, int *resplen2)
{
  int gotsomewhere, terrno, try, v_circuit, resplen, ns, n;

	if (statp->nscount == 0) {
		__set_errno (ESRCH);
		return (-1);
	}

	if (anssiz < (buf2 == NULL ? 1 : 2) * HFIXEDSZ) {
		__set_errno (EINVAL);
		return (-1);
	}

#ifdef USE_HOOKS
	if (__builtin_expect (statp->qhook || statp->rhook, 0)) {
		if (anssiz < MAXPACKET && ansp) {
			u_char *buf = malloc (MAXPACKET);
			if (buf == NULL)
				return (-1);
			memcpy (buf, ans, HFIXEDSZ);
			*ansp = buf;
			ans = buf;
			anssiz = MAXPACKET;
		}
	}
#endif

	DprintQ((statp->options & RES_DEBUG) || (statp->pfcode & RES_PRF_QUERY),
		(stdout, ";; res_send()\n"), buf, buflen);
	v_circuit = ((statp->options & RES_USEVC)
		     || buflen > PACKETSZ
		     || buflen2 > PACKETSZ);
	gotsomewhere = 0;
	terrno = ETIMEDOUT;

	/*
	 * If the ns_addr_list in the resolver context has changed, then
	 * invalidate our cached copy and the associated timing data.
	 */
	if (EXT(statp).nsinit) {
		int needclose = 0;

		if (EXT(statp).nscount != statp->nscount)
			needclose++;
		else
			for (ns = 0; ns < MAXNS; ns++) {
				unsigned int map = EXT(statp).nsmap[ns];
				if (map < MAXNS
				    && !sock_eq((struct sockaddr_in6 *)
						&statp->nsaddr_list[map],
						EXT(statp).nsaddrs[ns]))
				{
					needclose++;
					break;
				}
			}
		if (needclose)
			__res_iclose(statp, false);
	}

	/*
	 * Maybe initialize our private copy of the ns_addr_list.
	 */
	if (EXT(statp).nsinit == 0) {
		unsigned char map[MAXNS];

		memset (map, MAXNS, sizeof (map));
		for (n = 0; n < MAXNS; n++) {
			ns = EXT(statp).nsmap[n];
			if (ns < statp->nscount)
				map[ns] = n;
			else if (ns < MAXNS) {
				free(EXT(statp).nsaddrs[n]);
				EXT(statp).nsaddrs[n] = NULL;
				EXT(statp).nsmap[n] = MAXNS;
			}
		}
		n = statp->nscount;
		if (statp->nscount > EXT(statp).nscount)
			for (n = EXT(statp).nscount, ns = 0;
			     n < statp->nscount; n++) {
				while (ns < MAXNS
				       && EXT(statp).nsmap[ns] != MAXNS)
					ns++;
				if (ns == MAXNS)
					break;
				EXT(statp).nsmap[ns] = n;
				map[n] = ns++;
			}
		EXT(statp).nscount = n;
		for (ns = 0; ns < EXT(statp).nscount; ns++) {
			n = map[ns];
			if (EXT(statp).nsaddrs[n] == NULL)
				EXT(statp).nsaddrs[n] =
				    malloc(sizeof (struct sockaddr_in6));
			if (EXT(statp).nsaddrs[n] != NULL) {
				memset (mempcpy(EXT(statp).nsaddrs[n],
						&statp->nsaddr_list[n],
						sizeof (struct sockaddr_in)),
					'\0',
					sizeof (struct sockaddr_in6)
					- sizeof (struct sockaddr_in));
				EXT(statp).nssocks[n] = -1;
				n++;
			}
		}
		EXT(statp).nsinit = 1;
	}

	/*
	 * Some resolvers want to even out the load on their nameservers.
	 * Note that RES_BLAST overrides RES_ROTATE.
	 */
	if (__builtin_expect ((statp->options & RES_ROTATE) != 0, 0) &&
	    (statp->options & RES_BLAST) == 0) {
		struct sockaddr_in6 *ina;
		unsigned int map;

		n = 0;
		while (n < MAXNS && EXT(statp).nsmap[n] == MAXNS)
			n++;
		if (n < MAXNS) {
			ina = EXT(statp).nsaddrs[n];
			map = EXT(statp).nsmap[n];
			for (;;) {
				ns = n + 1;
				while (ns < MAXNS
				       && EXT(statp).nsmap[ns] == MAXNS)
					ns++;
				if (ns == MAXNS)
					break;
				EXT(statp).nsaddrs[n] = EXT(statp).nsaddrs[ns];
				EXT(statp).nsmap[n] = EXT(statp).nsmap[ns];
				n = ns;
			}
			EXT(statp).nsaddrs[n] = ina;
			EXT(statp).nsmap[n] = map;
		}
	}

	/*
	 * Send request, RETRY times, or until successful.
	 */
	for (try = 0; try < statp->retry; try++) {
	    for (ns = 0; ns < MAXNS; ns++)
	    {
#ifdef DEBUG
		char tmpbuf[40];
#endif
		struct sockaddr_in6 *nsap = EXT(statp).nsaddrs[ns];

		if (nsap == NULL)
			goto next_ns;
	    same_ns:
#ifdef USE_HOOKS
		if (__builtin_expect (statp->qhook != NULL, 0)) {
			int done = 0, loops = 0;

			do {
				res_sendhookact act;

				struct sockaddr_in *nsap4;
				nsap4 = (struct sockaddr_in *) nsap;
				act = (*statp->qhook)(&nsap4, &buf, &buflen,
						      ans, anssiz, &resplen);
				nsap = (struct sockaddr_in6 *) nsap4;
				switch (act) {
				case res_goahead:
					done = 1;
					break;
				case res_nextns:
					__res_iclose(statp, false);
					goto next_ns;
				case res_done:
					return (resplen);
				case res_modified:
					/* give the hook another try */
					if (++loops < 42) /*doug adams*/
						break;
					/*FALLTHROUGH*/
				case res_error:
					/*FALLTHROUGH*/
				default:
					return (-1);
				}
			} while (!done);
		}
#endif

		Dprint(statp->options & RES_DEBUG,
		       (stdout, ";; Querying server (# %d) address = %s\n",
			ns + 1, inet_ntop(nsap->sin6_family,
					  (nsap->sin6_family == AF_INET6
					   ? &nsap->sin6_addr
					   : &((struct sockaddr_in *) nsap)->sin_addr),
					  tmpbuf, sizeof (tmpbuf))));

		if (__builtin_expect (v_circuit, 0)) {
			/* Use VC; at most one attempt per server. */
			try = statp->retry;
			n = send_vc(statp, buf, buflen, buf2, buflen2,
				    &ans, &anssiz, &terrno,
				    ns, ansp, ansp2, nansp2, resplen2);
			if (n < 0)
				return (-1);
			if (n == 0 && (buf2 == NULL || *resplen2 == 0))
				goto next_ns;
		} else {
			/* Use datagrams. */
			n = send_dg(statp, buf, buflen, buf2, buflen2,
				    &ans, &anssiz, &terrno,
				    ns, &v_circuit, &gotsomewhere, ansp,
				    ansp2, nansp2, resplen2);
			if (n < 0)
				return (-1);
			if (n == 0 && (buf2 == NULL || *resplen2 == 0))
				goto next_ns;
			if (v_circuit)
			  // XXX Check whether both requests failed or
			  // XXX whether one has been answered successfully
				goto same_ns;
		}

		resplen = n;

		Dprint((statp->options & RES_DEBUG) ||
		       ((statp->pfcode & RES_PRF_REPLY) &&
			(statp->pfcode & RES_PRF_HEAD1)),
		       (stdout, ";; got answer:\n"));

		DprintQ((statp->options & RES_DEBUG) ||
			(statp->pfcode & RES_PRF_REPLY),
			(stdout, "%s", ""),
			ans, (resplen > anssiz) ? anssiz : resplen);
		if (buf2 != NULL) {
		  DprintQ((statp->options & RES_DEBUG) ||
			  (statp->pfcode & RES_PRF_REPLY),
			  (stdout, "%s", ""),
			  *ansp2, (*resplen2 > *nansp2) ? *nansp2 : *resplen2);
		}

		/*
		 * If we have temporarily opened a virtual circuit,
		 * or if we haven't been asked to keep a socket open,
		 * close the socket.
		 */
		if ((v_circuit && (statp->options & RES_USEVC) == 0) ||
		    (statp->options & RES_STAYOPEN) == 0) {
			__res_iclose(statp, false);
		}
#ifdef USE_HOOKS
		if (__builtin_expect (statp->rhook, 0)) {
			int done = 0, loops = 0;

			do {
				res_sendhookact act;

				act = (*statp->rhook)((struct sockaddr_in *)
						      nsap, buf, buflen,
						      ans, anssiz, &resplen);
				switch (act) {
				case res_goahead:
				case res_done:
					done = 1;
					break;
				case res_nextns:
					__res_iclose(statp, false);
					goto next_ns;
				case res_modified:
					/* give the hook another try */
					if (++loops < 42) /*doug adams*/
						break;
					/*FALLTHROUGH*/
				case res_error:
					/*FALLTHROUGH*/
				default:
					return (-1);
				}
			} while (!done);

		}
#endif
		return (resplen);
 next_ns: ;
	   } /*foreach ns*/
	} /*foreach retry*/
	__res_iclose(statp, false);
	if (!v_circuit) {
		if (!gotsomewhere)
			__set_errno (ECONNREFUSED);	/* no nameservers found */
		else
			__set_errno (ETIMEDOUT);	/* no answer obtained */
	} else
		__set_errno (terrno);
	return (-1);
}

int
res_nsend(res_state statp,
	  const u_char *buf, int buflen, u_char *ans, int anssiz)
{
  return __libc_res_nsend(statp, buf, buflen, NULL, 0, ans, anssiz,
			  NULL, NULL, NULL, NULL);
}
libresolv_hidden_def (res_nsend)

/* Private */

static int
send_vc(res_state statp,
	const u_char *buf, int buflen, const u_char *buf2, int buflen2,
	u_char **ansp, int *anssizp,
	int *terrno, int ns, u_char **anscp, u_char **ansp2, int *anssizp2,
	int *resplen2)
{
	const HEADER *hp = (HEADER *) buf;
	const HEADER *hp2 = (HEADER *) buf2;
	u_char *ans = *ansp;
	int orig_anssizp = *anssizp;
	// XXX REMOVE
	// int anssiz = *anssizp;
	HEADER *anhp = (HEADER *) ans;
	struct sockaddr_in6 *nsap = EXT(statp).nsaddrs[ns];
	int truncating, connreset, resplen, n;
	struct iovec iov[4];
	u_short len;
	u_short len2;
	u_char *cp;

	if (resplen2 != NULL)
	  *resplen2 = 0;
	connreset = 0;
 same_ns:
	truncating = 0;

	/* Are we still talking to whom we want to talk to? */
	if (statp->_vcsock >= 0 && (statp->_flags & RES_F_VC) != 0) {
		struct sockaddr_in6 peer;
		socklen_t size = sizeof peer;

		if (getpeername(statp->_vcsock,
				(struct sockaddr *)&peer, &size) < 0 ||
		    !sock_eq(&peer, nsap)) {
		  __res_iclose(statp, false);
			statp->_flags &= ~RES_F_VC;
		}
	}

	if (statp->_vcsock < 0 || (statp->_flags & RES_F_VC) == 0) {
		if (statp->_vcsock >= 0)
		  __res_iclose(statp, false);

		statp->_vcsock = socket(nsap->sin6_family, SOCK_STREAM, 0);
		if (statp->_vcsock < 0) {
			*terrno = errno;
			Perror(statp, stderr, "socket(vc)", errno);
			return (-1);
		}
		__set_errno (0);
		if (connect(statp->_vcsock, (struct sockaddr *)nsap,
			    nsap->sin6_family == AF_INET
			    ? sizeof (struct sockaddr_in)
			    : sizeof (struct sockaddr_in6)) < 0) {
			*terrno = errno;
			Aerror(statp, stderr, "connect/vc", errno,
			       (struct sockaddr *) nsap);
			__res_iclose(statp, false);
			return (0);
		}
		statp->_flags |= RES_F_VC;
	}

	/*
	 * Send length & message
	 */
	len = htons ((u_short) buflen);
	evConsIovec(&len, INT16SZ, &iov[0]);
	evConsIovec((void*)buf, buflen, &iov[1]);
	int niov = 2;
	ssize_t explen = INT16SZ + buflen;
	if (buf2 != NULL) {
		len2 = htons ((u_short) buflen2);
		evConsIovec(&len2, INT16SZ, &iov[2]);
		evConsIovec((void*)buf2, buflen2, &iov[3]);
		niov = 4;
		explen += INT16SZ + buflen2;
	}
	if (TEMP_FAILURE_RETRY (writev(statp->_vcsock, iov, niov)) != explen) {
		*terrno = errno;
		Perror(statp, stderr, "write failed", errno);
		__res_iclose(statp, false);
		return (0);
	}
	/*
	 * Receive length & response
	 */
	int recvresp1 = 0;
	int recvresp2 = buf2 == NULL;
	uint16_t rlen16;
 read_len:
	cp = (u_char *)&rlen16;
	len = sizeof(rlen16);
	while ((n = TEMP_FAILURE_RETRY (read(statp->_vcsock, cp,
					     (int)len))) > 0) {
		cp += n;
		if ((len -= n) <= 0)
			break;
	}
	if (n <= 0) {
		*terrno = errno;
		Perror(statp, stderr, "read failed", errno);
		__res_iclose(statp, false);
		/*
		 * A long running process might get its TCP
		 * connection reset if the remote server was
		 * restarted.  Requery the server instead of
		 * trying a new one.  When there is only one
		 * server, this means that a query might work
		 * instead of failing.  We only allow one reset
		 * per query to prevent looping.
		 */
		if (*terrno == ECONNRESET && !connreset) {
			connreset = 1;
			goto same_ns;
		}
		return (0);
	}
	int rlen = ntohs (rlen16);

	int *thisanssizp;
	u_char **thisansp;
	int *thisresplenp;
	if ((recvresp1 | recvresp2) == 0 || buf2 == NULL) {
		thisanssizp = anssizp;
		thisansp = anscp ?: ansp;
		assert (anscp != NULL || ansp2 == NULL);
		thisresplenp = &resplen;
	} else {
		if (*anssizp != MAXPACKET) {
			/* No buffer allocated for the first
			   reply.  We can try to use the rest
			   of the user-provided buffer.  */
#ifdef _STRING_ARCH_unaligned
			*anssizp2 = orig_anssizp - resplen;
			*ansp2 = *ansp + resplen;
#else
			int aligned_resplen
			  = ((resplen + __alignof__ (HEADER) - 1)
			     & ~(__alignof__ (HEADER) - 1));
			*anssizp2 = orig_anssizp - aligned_resplen;
			*ansp2 = *ansp + aligned_resplen;
#endif
		} else {
			/* The first reply did not fit into the
			   user-provided buffer.  Maybe the second
			   answer will.  */
			*anssizp2 = orig_anssizp;
			*ansp2 = *ansp;
		}

		thisanssizp = anssizp2;
		thisansp = ansp2;
		thisresplenp = resplen2;
	}
	anhp = (HEADER *) *thisansp;

	*thisresplenp = rlen;
	if (rlen > *thisanssizp) {
		/* Yes, we test ANSCP here.  If we have two buffers
		   both will be allocatable.  */
		if (__builtin_expect (anscp != NULL, 1)) {
			u_char *newp = malloc (MAXPACKET);
			if (newp == NULL) {
				*terrno = ENOMEM;
				__res_iclose(statp, false);
				return (0);
			}
			*thisanssizp = MAXPACKET;
			*thisansp = newp;
			anhp = (HEADER *) newp;
			len = rlen;
		} else {
			Dprint(statp->options & RES_DEBUG,
				(stdout, ";; response truncated\n")
			);
			truncating = 1;
			len = *thisanssizp;
		}
	} else
		len = rlen;

	if (__builtin_expect (len < HFIXEDSZ, 0)) {
		/*
		 * Undersized message.
		 */
		Dprint(statp->options & RES_DEBUG,
		       (stdout, ";; undersized: %d\n", len));
		*terrno = EMSGSIZE;
		__res_iclose(statp, false);
		return (0);
	}

	cp = *thisansp;
	while (len != 0 && (n = read(statp->_vcsock, (char *)cp, (int)len)) > 0){
		cp += n;
		len -= n;
	}
	if (__builtin_expect (n <= 0, 0)) {
		*terrno = errno;
		Perror(statp, stderr, "read(vc)", errno);
		__res_iclose(statp, false);
		return (0);
	}
	if (__builtin_expect (truncating, 0)) {
		/*
		 * Flush rest of answer so connection stays in synch.
		 */
		anhp->tc = 1;
		len = rlen - *thisanssizp;
		while (len != 0) {
			char junk[PACKETSZ];

			n = read(statp->_vcsock, junk,
				 (len > sizeof junk) ? sizeof junk : len);
			if (n > 0)
				len -= n;
			else
				break;
		}
	}
	/*
	 * If the calling applicating has bailed out of
	 * a previous call and failed to arrange to have
	 * the circuit closed or the server has got
	 * itself confused, then drop the packet and
	 * wait for the correct one.
	 */
	if ((recvresp1 || hp->id != anhp->id)
	    && (recvresp2 || hp2->id != anhp->id)) {
		DprintQ((statp->options & RES_DEBUG) ||
			(statp->pfcode & RES_PRF_REPLY),
			(stdout, ";; old answer (unexpected):\n"),
			*thisansp,
			(rlen > *thisanssizp) ? *thisanssizp: rlen);
		goto read_len;
	}

	/* Mark which reply we received.  */
	if (recvresp1 == 0 && hp->id == anhp->id)
	  recvresp1 = 1;
	else
	  recvresp2 = 1;
	/* Repeat waiting if we have a second answer to arrive.  */
	if ((recvresp1 & recvresp2) == 0)
		goto read_len;

	/*
	 * All is well, or the error is fatal.  Signal that the
	 * next nameserver ought not be tried.
	 */
	return resplen;
}

static int
reopen (res_state statp, int *terrno, int ns)
{
	if (EXT(statp).nssocks[ns] == -1) {
		struct sockaddr *nsap
		  = (struct sockaddr *) EXT(statp).nsaddrs[ns];
		socklen_t slen;

		/* only try IPv6 if IPv6 NS and if not failed before */
		if (nsap->sa_family == AF_INET6 && !statp->ipv6_unavail) {
			if (__builtin_expect (__have_o_nonblock >= 0, 1)) {
				EXT(statp).nssocks[ns] =
				  socket(PF_INET6, SOCK_DGRAM|SOCK_NONBLOCK,
					 0);
#ifndef __ASSUME_SOCK_CLOEXEC
				if (__have_o_nonblock == 0)
					__have_o_nonblock
					  = (EXT(statp).nssocks[ns] == -1
					     && errno == EINVAL ? -1 : 1);
#endif
			}
			if (__builtin_expect (__have_o_nonblock < 0, 0))
				EXT(statp).nssocks[ns] =
				  socket(PF_INET6, SOCK_DGRAM, 0);
			if (EXT(statp).nssocks[ns] < 0)
			    statp->ipv6_unavail = errno == EAFNOSUPPORT;
			slen = sizeof (struct sockaddr_in6);
		} else if (nsap->sa_family == AF_INET) {
			if (__builtin_expect (__have_o_nonblock >= 0, 1)) {
				EXT(statp).nssocks[ns]
				  = socket(PF_INET, SOCK_DGRAM|SOCK_NONBLOCK,
					   0);
#ifndef __ASSUME_SOCK_CLOEXEC
				if (__have_o_nonblock == 0)
					__have_o_nonblock
					  = (EXT(statp).nssocks[ns] == -1
					     && errno == EINVAL ? -1 : 1);
#endif
			}
			if (__builtin_expect (__have_o_nonblock < 0, 0))
				EXT(statp).nssocks[ns]
				  = socket(PF_INET, SOCK_DGRAM, 0);
			slen = sizeof (struct sockaddr_in);
		}
		if (EXT(statp).nssocks[ns] < 0) {
			*terrno = errno;
			Perror(statp, stderr, "socket(dg)", errno);
			return (-1);
		}

		/*
		 * On a 4.3BSD+ machine (client and server,
		 * actually), sending to a nameserver datagram
		 * port with no nameserver will cause an
		 * ICMP port unreachable message to be returned.
		 * If our datagram socket is "connected" to the
		 * server, we get an ECONNREFUSED error on the next
		 * socket operation, and select returns if the
		 * error message is received.  We can thus detect
		 * the absence of a nameserver without timing out.
		 */
		if (connect(EXT(statp).nssocks[ns], nsap, slen) < 0) {
			Aerror(statp, stderr, "connect(dg)", errno, nsap);
			__res_iclose(statp, false);
			return (0);
		}
		if (__builtin_expect (__have_o_nonblock < 0, 0)) {
			/* Make socket non-blocking.  */
			int fl = __fcntl (EXT(statp).nssocks[ns], F_GETFL);
			if  (fl != -1)
				__fcntl (EXT(statp).nssocks[ns], F_SETFL,
					 fl | O_NONBLOCK);
			Dprint(statp->options & RES_DEBUG,
			       (stdout, ";; new DG socket\n"))
		}
	}

	return 1;
}

static int
send_dg(res_state statp,
	const u_char *buf, int buflen, const u_char *buf2, int buflen2,
	u_char **ansp, int *anssizp,
	int *terrno, int ns, int *v_circuit, int *gotsomewhere, u_char **anscp,
	u_char **ansp2, int *anssizp2, int *resplen2)
{
	const HEADER *hp = (HEADER *) buf;
	const HEADER *hp2 = (HEADER *) buf2;
	u_char *ans = *ansp;
	int orig_anssizp = *anssizp;
	struct timespec now, timeout, finish;
	struct pollfd pfd[1];
	int ptimeout;
	struct sockaddr_in6 from;
	int resplen = 0;
	int n;

	/*
	 * Compute time for the total operation.
	 */
	int seconds = (statp->retrans << ns);
	if (ns > 0)
		seconds /= statp->nscount;
	if (seconds <= 0)
		seconds = 1;
	bool single_request_reopen = (statp->options & RES_SNGLKUPREOP) != 0;
	bool single_request = (((statp->options & RES_SNGLKUP) != 0)
			       | single_request_reopen);
	int save_gotsomewhere = *gotsomewhere;

	int retval;
 retry_reopen:
	retval = reopen (statp, terrno, ns);
	if (retval <= 0)
		return retval;
 retry:
	evNowTime(&now);
	evConsTime(&timeout, seconds, 0);
	evAddTime(&finish, &now, &timeout);
	int need_recompute = 0;
	int nwritten = 0;
	int recvresp1 = 0;
	int recvresp2 = buf2 == NULL;
	pfd[0].fd = EXT(statp).nssocks[ns];
	pfd[0].events = POLLOUT;
	if (resplen2 != NULL)
	  *resplen2 = 0;
 wait:
	if (need_recompute) {
	recompute_resend:
		evNowTime(&now);
		if (evCmpTime(finish, now) <= 0) {
		poll_err_out:
			Perror(statp, stderr, "poll", errno);
		err_out:
			__res_iclose(statp, false);
			return (0);
		}
		evSubTime(&timeout, &finish, &now);
		need_recompute = 0;
	}
	/* Convert struct timespec in milliseconds.  */
	ptimeout = timeout.tv_sec * 1000 + timeout.tv_nsec / 1000000;

	n = 0;
	if (nwritten == 0)
	  n = __poll (pfd, 1, 0);
	if (__builtin_expect (n == 0, 0)) {
		n = __poll (pfd, 1, ptimeout);
		need_recompute = 1;
	}
	if (n == 0) {
		Dprint(statp->options & RES_DEBUG, (stdout, ";; timeout\n"));
		if (resplen > 1 && (recvresp1 || (buf2 != NULL && recvresp2)))
		  {
		    /* There are quite a few broken name servers out
		       there which don't handle two outstanding
		       requests from the same source.  There are also
		       broken firewall settings.  If we time out after
		       having received one answer switch to the mode
		       where we send the second request only once we
		       have received the first answer.  */
		    if (!single_request)
		      {
			statp->options |= RES_SNGLKUP;
			single_request = true;
			*gotsomewhere = save_gotsomewhere;
			goto retry;
		      }
		    else if (!single_request_reopen)
		      {
			statp->options |= RES_SNGLKUPREOP;
			single_request_reopen = true;
			*gotsomewhere = save_gotsomewhere;
			__res_iclose (statp, false);
			goto retry_reopen;
		      }

		    *resplen2 = 1;
		    return resplen;
		  }

		*gotsomewhere = 1;
		return (0);
	}
	if (n < 0) {
		if (errno == EINTR)
			goto recompute_resend;

		goto poll_err_out;
	}
	__set_errno (0);
	if (pfd[0].revents & POLLOUT) {
#ifndef __ASSUME_SENDMMSG
		static int have_sendmmsg;
#else
# define have_sendmmsg 1
#endif
		if (have_sendmmsg >= 0 && nwritten == 0 && buf2 != NULL
		    && !single_request)
		  {
		    struct iovec iov[2];
		    struct mmsghdr reqs[2];
		    reqs[0].msg_hdr.msg_name = NULL;
		    reqs[0].msg_hdr.msg_namelen = 0;
		    reqs[0].msg_hdr.msg_iov = &iov[0];
		    reqs[0].msg_hdr.msg_iovlen = 1;
		    iov[0].iov_base = (void *) buf;
		    iov[0].iov_len = buflen;
		    reqs[0].msg_hdr.msg_control = NULL;
		    reqs[0].msg_hdr.msg_controllen = 0;

		    reqs[1].msg_hdr.msg_name = NULL;
		    reqs[1].msg_hdr.msg_namelen = 0;
		    reqs[1].msg_hdr.msg_iov = &iov[1];
		    reqs[1].msg_hdr.msg_iovlen = 1;
		    iov[1].iov_base = (void *) buf2;
		    iov[1].iov_len = buflen2;
		    reqs[1].msg_hdr.msg_control = NULL;
		    reqs[1].msg_hdr.msg_controllen = 0;

		    int ndg = __sendmmsg (pfd[0].fd, reqs, 2, MSG_NOSIGNAL);
		    if (__builtin_expect (ndg == 2, 1))
		      {
			if (reqs[0].msg_len != buflen
			    || reqs[1].msg_len != buflen2)
			  goto fail_sendmmsg;

			pfd[0].events = POLLIN;
			nwritten += 2;
		      }
		    else if (ndg == 1 && reqs[0].msg_len == buflen)
		      goto just_one;
		    else if (ndg < 0 && (errno == EINTR || errno == EAGAIN))
		      goto recompute_resend;
		    else
		      {
#ifndef __ASSUME_SENDMMSG
			if (__builtin_expect (have_sendmmsg == 0, 0))
			  {
			    if (ndg < 0 && errno == ENOSYS)
			      {
				have_sendmmsg = -1;
				goto try_send;
			      }
			    have_sendmmsg = 1;
			  }
#endif

		      fail_sendmmsg:
			Perror(statp, stderr, "sendmmsg", errno);
			goto err_out;
		      }
		  }
		else
		  {
		    ssize_t sr;
#ifndef __ASSUME_SENDMMSG
		  try_send:
#endif
		    if (nwritten != 0)
		      sr = send (pfd[0].fd, buf2, buflen2, MSG_NOSIGNAL);
		    else
		      sr = send (pfd[0].fd, buf, buflen, MSG_NOSIGNAL);

		    if (sr != (nwritten != 0 ? buflen2 : buflen)) {
		      if (errno == EINTR || errno == EAGAIN)
			goto recompute_resend;
		      Perror(statp, stderr, "send", errno);
		      goto err_out;
		    }
		  just_one:
		    if (nwritten != 0 || buf2 == NULL || single_request)
		      pfd[0].events = POLLIN;
		    else
		      pfd[0].events = POLLIN | POLLOUT;
		    ++nwritten;
		  }
		goto wait;
	} else if (pfd[0].revents & POLLIN) {
		int *thisanssizp;
		u_char **thisansp;
		int *thisresplenp;

		if ((recvresp1 | recvresp2) == 0 || buf2 == NULL) {
			thisanssizp = anssizp;
			thisansp = anscp ?: ansp;
			assert (anscp != NULL || ansp2 == NULL);
			thisresplenp = &resplen;
		} else {
			if (*anssizp != MAXPACKET) {
				/* No buffer allocated for the first
				   reply.  We can try to use the rest
				   of the user-provided buffer.  */
#ifdef _STRING_ARCH_unaligned
				*anssizp2 = orig_anssizp - resplen;
				*ansp2 = *ansp + resplen;
#else
				int aligned_resplen
				  = ((resplen + __alignof__ (HEADER) - 1)
				     & ~(__alignof__ (HEADER) - 1));
				*anssizp2 = orig_anssizp - aligned_resplen;
				*ansp2 = *ansp + aligned_resplen;
#endif
			} else {
				/* The first reply did not fit into the
				   user-provided buffer.  Maybe the second
				   answer will.  */
				*anssizp2 = orig_anssizp;
				*ansp2 = *ansp;
			}

			thisanssizp = anssizp2;
			thisansp = ansp2;
			thisresplenp = resplen2;
		}

		if (*thisanssizp < MAXPACKET
		    /* Yes, we test ANSCP here.  If we have two buffers
		       both will be allocatable.  */
		    && anscp
		    && (ioctl (pfd[0].fd, FIONREAD, thisresplenp) < 0
			|| *thisanssizp < *thisresplenp)) {
			u_char *newp = malloc (MAXPACKET);
			if (newp != NULL) {
				*anssizp = MAXPACKET;
				*thisansp = ans = newp;
			}
		}
		HEADER *anhp = (HEADER *) *thisansp;
		socklen_t fromlen = sizeof(struct sockaddr_in6);
		assert (sizeof(from) <= fromlen);
		*thisresplenp = recvfrom(pfd[0].fd, (char*)*thisansp,
					 *thisanssizp, 0,
					(struct sockaddr *)&from, &fromlen);
		if (__builtin_expect (*thisresplenp <= 0, 0)) {
			if (errno == EINTR || errno == EAGAIN) {
				need_recompute = 1;
				goto wait;
			}
			Perror(statp, stderr, "recvfrom", errno);
			goto err_out;
		}
		*gotsomewhere = 1;
		if (__builtin_expect (*thisresplenp < HFIXEDSZ, 0)) {
			/*
			 * Undersized message.
			 */
			Dprint(statp->options & RES_DEBUG,
			       (stdout, ";; undersized: %d\n",
				*thisresplenp));
			*terrno = EMSGSIZE;
			goto err_out;
		}
		if ((recvresp1 || hp->id != anhp->id)
		    && (recvresp2 || hp2->id != anhp->id)) {
			/*
			 * response from old query, ignore it.
			 * XXX - potential security hazard could
			 *	 be detected here.
			 */
			DprintQ((statp->options & RES_DEBUG) ||
				(statp->pfcode & RES_PRF_REPLY),
				(stdout, ";; old answer:\n"),
				*thisansp,
				(*thisresplenp > *thisanssizp)
				? *thisanssizp : *thisresplenp);
			goto wait;
		}
		if (!(statp->options & RES_INSECURE1) &&
		    !res_ourserver_p(statp, &from)) {
			/*
			 * response from wrong server? ignore it.
			 * XXX - potential security hazard could
			 *	 be detected here.
			 */
			DprintQ((statp->options & RES_DEBUG) ||
				(statp->pfcode & RES_PRF_REPLY),
				(stdout, ";; not our server:\n"),
				*thisansp,
				(*thisresplenp > *thisanssizp)
				? *thisanssizp : *thisresplenp);
			goto wait;
		}
#ifdef RES_USE_EDNS0
		if (anhp->rcode == FORMERR
		    && (statp->options & RES_USE_EDNS0) != 0U) {
			/*
			 * Do not retry if the server does not understand
			 * EDNS0.  The case has to be captured here, as
			 * FORMERR packet do not carry query section, hence
			 * res_queriesmatch() returns 0.
			 */
			DprintQ(statp->options & RES_DEBUG,
				(stdout,
				 "server rejected query with EDNS0:\n"),
				*thisansp,
				(*thisresplenp > *thisanssizp)
				? *thisanssizp : *thisresplenp);
			/* record the error */
			statp->_flags |= RES_F_EDNS0ERR;
			goto err_out;
	}
#endif
		if (!(statp->options & RES_INSECURE2)
		    && (recvresp1 || !res_queriesmatch(buf, buf + buflen,
						       *thisansp,
						       *thisansp
						       + *thisanssizp))
		    && (recvresp2 || !res_queriesmatch(buf2, buf2 + buflen2,
						       *thisansp,
						       *thisansp
						       + *thisanssizp))) {
			/*
			 * response contains wrong query? ignore it.
			 * XXX - potential security hazard could
			 *	 be detected here.
			 */
			DprintQ((statp->options & RES_DEBUG) ||
				(statp->pfcode & RES_PRF_REPLY),
				(stdout, ";; wrong query name:\n"),
				*thisansp,
				(*thisresplenp > *thisanssizp)
				? *thisanssizp : *thisresplenp);
			goto wait;
		}
		if (anhp->rcode == SERVFAIL ||
		    anhp->rcode == NOTIMP ||
		    anhp->rcode == REFUSED) {
			DprintQ(statp->options & RES_DEBUG,
				(stdout, "server rejected query:\n"),
				*thisansp,
				(*thisresplenp > *thisanssizp)
				? *thisanssizp : *thisresplenp);

			if (recvresp1 || (buf2 != NULL && recvresp2)) {
			  *resplen2 = 0;
			  return resplen;
			}
			if (buf2 != NULL)
			  {
			    /* No data from the first reply.  */
			    resplen = 0;
			    /* We are waiting for a possible second reply.  */
			    if (hp->id == anhp->id)
			      recvresp1 = 1;
			    else
			      recvresp2 = 1;

			    goto wait;
			  }

		next_ns:
			__res_iclose(statp, false);
			/* don't retry if called from dig */
			if (!statp->pfcode)
				return (0);
		}
		if (anhp->rcode == NOERROR && anhp->ancount == 0
		    && anhp->aa == 0 && anhp->ra == 0 && anhp->arcount == 0) {
			DprintQ(statp->options & RES_DEBUG,
				(stdout, "referred query:\n"),
				*thisansp,
				(*thisresplenp > *thisanssizp)
				? *thisanssizp : *thisresplenp);
			goto next_ns;
		}
		if (!(statp->options & RES_IGNTC) && anhp->tc) {
			/*
			 * To get the rest of answer,
			 * use TCP with same server.
			 */
			Dprint(statp->options & RES_DEBUG,
			       (stdout, ";; truncated answer\n"));
			*v_circuit = 1;
			__res_iclose(statp, false);
			// XXX if we have received one reply we could
			// XXX use it and not repeat it over TCP...
			return (1);
		}
		/* Mark which reply we received.  */
		if (recvresp1 == 0 && hp->id == anhp->id)
			recvresp1 = 1;
		else
			recvresp2 = 1;
		/* Repeat waiting if we have a second answer to arrive.  */
		if ((recvresp1 & recvresp2) == 0) {
			if (single_request) {
				pfd[0].events = POLLOUT;
				if (single_request_reopen) {
					__res_iclose (statp, false);
					retval = reopen (statp, terrno, ns);
					if (retval <= 0)
						return retval;
				}
			}
			goto wait;
		}
		/*
		 * All is well, or the error is fatal.  Signal that the
		 * next nameserver ought not be tried.
		 */
		return (resplen);
	} else if (pfd[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
		/* Something went wrong.  We can stop trying.  */
		goto err_out;
	}
	else {
		/* poll should not have returned > 0 in this case.  */
		abort ();
	}
}

#ifdef DEBUG
static void
Aerror(const res_state statp, FILE *file, const char *string, int error,
       const struct sockaddr *address)
{
	int save = errno;

	if ((statp->options & RES_DEBUG) != 0) {
		char tmp[sizeof "xxxx.xxxx.xxxx.255.255.255.255"];

		fprintf(file, "res_send: %s ([%s].%u): %s\n",
			string,
			(address->sa_family == AF_INET
			 ? inet_ntop(address->sa_family,
				     &((const struct sockaddr_in *) address)->sin_addr,
				     tmp, sizeof tmp)
			 : inet_ntop(address->sa_family,
				     &((const struct sockaddr_in6 *) address)->sin6_addr,
				     tmp, sizeof tmp)),
			(address->sa_family == AF_INET
			 ? ntohs(((struct sockaddr_in *) address)->sin_port)
			 : address->sa_family == AF_INET6
			 ? ntohs(((struct sockaddr_in6 *) address)->sin6_port)
			 : 0),
			strerror(error));
	}
	__set_errno (save);
}

static void
Perror(const res_state statp, FILE *file, const char *string, int error) {
	int save = errno;

	if ((statp->options & RES_DEBUG) != 0)
		fprintf(file, "res_send: %s: %s\n",
			string, strerror(error));
	__set_errno (save);
}
#endif

static int
sock_eq(struct sockaddr_in6 *a1, struct sockaddr_in6 *a2) {
	if (a1->sin6_family == a2->sin6_family) {
		if (a1->sin6_family == AF_INET)
			return ((((struct sockaddr_in *)a1)->sin_port ==
				 ((struct sockaddr_in *)a2)->sin_port) &&
				(((struct sockaddr_in *)a1)->sin_addr.s_addr ==
				 ((struct sockaddr_in *)a2)->sin_addr.s_addr));
		else
			return ((a1->sin6_port == a2->sin6_port) &&
				!memcmp(&a1->sin6_addr, &a2->sin6_addr,
					sizeof (struct in6_addr)));
	}
	if (a1->sin6_family == AF_INET) {
		struct sockaddr_in6 *sap = a1;
		a1 = a2;
		a2 = sap;
	} /* assumes that AF_INET and AF_INET6 are the only possibilities */
	return ((a1->sin6_port == ((struct sockaddr_in *)a2)->sin_port) &&
		IN6_IS_ADDR_V4MAPPED(&a1->sin6_addr) &&
		(a1->sin6_addr.s6_addr32[3] ==
		 ((struct sockaddr_in *)a2)->sin_addr.s_addr));
}
