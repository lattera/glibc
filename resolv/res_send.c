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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/uio.h>
#ifdef _LIBC
#include <sys/poll.h>
#endif

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef _LIBC
#include <isc/eventlib.h>
#else

/* From ev_streams.c.  */

static inline struct iovec
evConsIovec(void *buf, size_t cnt) {
	struct iovec ret;

	memset(&ret, 0xf5, sizeof ret);
	ret.iov_base = buf;
	ret.iov_len = cnt;
	return (ret);
}

/* From ev_timers.c.  */

#define BILLION 1000000000

static inline struct timespec
evTimeSpec(struct timeval tv) {
        struct timespec ts;

        ts.tv_sec = tv.tv_sec;
        ts.tv_nsec = tv.tv_usec * 1000;
        return (ts);
}

static inline struct timespec
evConsTime(time_t sec, long nsec) {
	struct timespec x;

	x.tv_sec = sec;
	x.tv_nsec = nsec;
	return (x);
}

static inline struct timespec
evAddTime(struct timespec addend1, struct timespec addend2) {
	struct timespec x;

	x.tv_sec = addend1.tv_sec + addend2.tv_sec;
	x.tv_nsec = addend1.tv_nsec + addend2.tv_nsec;
	if (x.tv_nsec >= BILLION) {
		x.tv_sec++;
		x.tv_nsec -= BILLION;
	}
	return (x);
}

static inline struct timespec
evSubTime(struct timespec minuend, struct timespec subtrahend) {
	struct timespec x;

	x.tv_sec = minuend.tv_sec - subtrahend.tv_sec;
	if (minuend.tv_nsec >= subtrahend.tv_nsec)
		x.tv_nsec = minuend.tv_nsec - subtrahend.tv_nsec;
	else {
		x.tv_nsec = BILLION - subtrahend.tv_nsec + minuend.tv_nsec;
		x.tv_sec--;
	}
	return (x);
}

static inline int
evCmpTime(struct timespec a, struct timespec b) {
	long x = a.tv_sec - b.tv_sec;

	if (x == 0L)
		x = a.tv_nsec - b.tv_nsec;
	return (x < 0L ? (-1) : x > 0L ? (1) : (0));
}

static inline struct timespec
evNowTime() {
	struct timeval now;

	if (gettimeofday(&now, NULL) < 0)
		return (evConsTime(0, 0));
	return (evTimeSpec(now));
}

#endif

/* Options.  Leave them on. */
/* #undef DEBUG */
#include "res_debug.h"

#define EXT(res) ((res)->_u._ext)

#ifndef _LIBC
static const int highestFD = FD_SETSIZE - 1;
#endif

/* Forward. */

static int		send_vc(res_state, const u_char *, int,
				u_char *, int, int *, int);
static int		send_dg(res_state, const u_char *, int,
				u_char *, int, int *, int,
				int *, int *);
#ifdef DEBUG
static void		Aerror(const res_state, FILE *, const char *, int,
			       struct sockaddr_in);
static void		Perror(const res_state, FILE *, const char *, int);
#endif
#ifdef _LIBC
static int		sock_eq(struct sockaddr_in6 *, struct sockaddr_in6 *);
#else
static int		sock_eq(struct sockaddr_in *, struct sockaddr_in *);
#endif
#ifdef NEED_PSELECT
static int		pselect(int, void *, void *, void *,
				struct timespec *,
				const sigset_t *);
#endif

/* Reachover. */

#ifdef _LIBC
static void convaddr4to6(struct sockaddr_in6 *sa);
#endif
void res_pquery(const res_state, const u_char *, int, FILE *);

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
#ifdef _LIBC
res_ourserver_p(const res_state statp, const struct sockaddr_in6 *inp)
#else
res_ourserver_p(const res_state statp, const struct sockaddr_in *inp)
#endif
{
	int ns;

#ifdef _LIBC
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
#else
	struct sockaddr_in ina = *inp;
	for (ns = 0; ns < statp->nscount; ns++) {
		const struct sockaddr_in *srv = &statp->nsaddr_list[ns];

		if (srv->sin_family == ina.sin_family &&
		    srv->sin_port == ina.sin_port &&
		    (srv->sin_addr.s_addr == INADDR_ANY ||
		     srv->sin_addr.s_addr == ina.sin_addr.s_addr))
			return (1);
	}
#endif
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
		ttype = ns_get16(cp); cp += INT16SZ;
		tclass = ns_get16(cp); cp += INT16SZ;
		if (ttype == type && tclass == class &&
		    ns_samename(tname, name) == 1)
			return (1);
	}
	return (0);
}

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
	const u_char *cp = buf1 + HFIXEDSZ;
	int qdcount = ntohs(((HEADER*)buf1)->qdcount);

	if (buf1 + HFIXEDSZ > eom1 || buf2 + HFIXEDSZ > eom2)
		return (-1);

	/*
	 * Only header section present in replies to
	 * dynamic update packets.
	 */
	if ((((HEADER *)buf1)->opcode == ns_o_update) &&
	    (((HEADER *)buf2)->opcode == ns_o_update))
		return (1);

	if (qdcount != ntohs(((HEADER*)buf2)->qdcount))
		return (0);
	while (qdcount-- > 0) {
		char tname[MAXDNAME+1];
		int n, ttype, tclass;

		n = dn_expand(buf1, eom1, cp, tname, sizeof tname);
		if (n < 0)
			return (-1);
		cp += n;
		if (cp + 2 * INT16SZ > eom1)
			return (-1);
		ttype = ns_get16(cp);	cp += INT16SZ;
		tclass = ns_get16(cp); cp += INT16SZ;
		if (!res_nameinquery(tname, ttype, tclass, buf2, eom2))
			return (0);
	}
	return (1);
}

int
res_nsend(res_state statp,
	  const u_char *buf, int buflen, u_char *ans, int anssiz)
{
	int gotsomewhere, terrno, try, v_circuit, resplen, ns, n;

	if (statp->nscount == 0) {
		__set_errno (ESRCH);
		return (-1);
	}
	if (anssiz < HFIXEDSZ) {
		__set_errno (EINVAL);
		return (-1);
	}
	DprintQ((statp->options & RES_DEBUG) || (statp->pfcode & RES_PRF_QUERY),
		(stdout, ";; res_send()\n"), buf, buflen);
	v_circuit = (statp->options & RES_USEVC) || buflen > PACKETSZ;
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
			for (ns = 0; ns < statp->nscount; ns++)
#ifdef _LIBC
				if (!sock_eq((struct sockaddr_in6 *)
					     &statp->nsaddr_list[ns],
					     EXT(statp).nsaddrs[ns]))
#else
				if (!sock_eq(&statp->nsaddr_list[ns],
					     &EXT(statp).nsaddrs[ns]))
#endif
				{
					needclose++;
					break;
				}
		if (needclose)
			res_nclose(statp);
	}

	/*
	 * Maybe initialize our private copy of the ns_addr_list.
	 */
	if (EXT(statp).nsinit == 0) {
#ifdef _LIBC
		n = 0;
#endif
		for (ns = 0; ns < statp->nscount; ns++) {
#ifdef _LIBC
			/* find a hole */
		  	while ((n < MAXNS) &&
			    (EXT(statp).nsaddrs[n] != NULL) &&
			    (EXT(statp).nsaddrs[n]->sin6_family == AF_INET6) &&
			    !IN6_IS_ADDR_V4MAPPED(
			        &EXT(statp).nsaddrs[n]->sin6_addr))
				n++;
			if (n == MAXNS)
				break;

			if (EXT(statp).nsaddrs[n] == NULL)
				EXT(statp).nsaddrs[n] =
				    malloc(sizeof (struct sockaddr_in6));
			if (EXT(statp).nsaddrs[n] != NULL) {
				memcpy(EXT(statp).nsaddrs[n],
				       &statp->nsaddr_list[ns],
				       sizeof (struct sockaddr_in));
				EXT(statp).nstimes[n] = RES_MAXTIME;
				EXT(statp).nssocks[n] = -1;
				n++;
			}
#else
			EXT(statp).nsaddrs[ns] = statp->nsaddr_list[ns];
			EXT(statp).nstimes[ns] = RES_MAXTIME;
			EXT(statp).nssocks[ns] = -1;
#endif
		}
		EXT(statp).nscount = statp->nscount;
		EXT(statp).nsinit = 1;
#ifdef _LIBC
		/* If holes left, free memory and set to NULL */
		while (n < MAXNS) {
			if ((EXT(statp).nsaddrs[n] != NULL) &&
			    ((EXT(statp).nsaddrs[n]->sin6_family != AF_INET6)
			    || IN6_IS_ADDR_V4MAPPED(
				   &EXT(statp).nsaddrs[n]->sin6_addr))) {
				free(EXT(statp).nsaddrs[n]);
				EXT(statp).nsaddrs[n] = NULL;
			}
			n++;
		}
#endif
	}

	/*
	 * Some resolvers want to even out the load on their nameservers.
	 * Note that RES_BLAST overrides RES_ROTATE.
	 */
	if ((statp->options & RES_ROTATE) != 0 &&
	    (statp->options & RES_BLAST) == 0) {
#ifdef _LIBC
		struct sockaddr_in6 *ina;
		int lastns = statp->nscount + EXT(statp).nscount6 - 1;

		ina = EXT(statp).nsaddrs[0];
		for (ns = 0; ns < lastns; ns++)
			EXT(statp).nsaddrs[ns] = EXT(statp).nsaddrs[ns + 1];
		EXT(statp).nsaddrs[lastns] = ina;
#else
		struct sockaddr_in ina;
		int lastns = statp->nscount - 1;

		ina = statp->nsaddr_list[0];
		for (ns = 0; ns < lastns; ns++)
			statp->nsaddr_list[ns] = statp->nsaddr_list[ns + 1];
		statp->nsaddr_list[lastns] = ina;
#endif
	}

	/*
	 * Send request, RETRY times, or until successful.
	 */
	for (try = 0; try < statp->retry; try++) {
#ifdef _LIBC
	    for (ns = 0; ns < MAXNS; ns++)
#else
	    for (ns = 0; ns < statp->nscount; ns++)
#endif
	    {
#ifdef _LIBC
		struct sockaddr_in6 *nsap = EXT(statp).nsaddrs[ns];

		if (nsap == NULL)
			goto next_ns;
#else
		struct sockaddr_in *nsap = &statp->nsaddr_list[ns];
#endif
 same_ns:
		if (statp->qhook) {
			int done = 0, loops = 0;

			do {
				res_sendhookact act;

#ifdef _LIBC
				act = (*statp->qhook)((struct sockaddr_in **)
						      &nsap, &buf, &buflen,
						      ans, anssiz, &resplen);
#else
				act = (*statp->qhook)(&nsap, &buf, &buflen,
						      ans, anssiz, &resplen);
#endif
				switch (act) {
				case res_goahead:
					done = 1;
					break;
				case res_nextns:
					res_nclose(statp);
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

		Dprint(statp->options & RES_DEBUG,
		       (stdout, ";; Querying server (# %d) address = %s\n",
			ns + 1, inet_ntoa(nsap->sin_addr)));

		if (v_circuit) {
			/* Use VC; at most one attempt per server. */
			try = statp->retry;
			n = send_vc(statp, buf, buflen, ans, anssiz, &terrno,
				    ns);
			if (n < 0)
				return (-1);
			if (n == 0)
				goto next_ns;
			resplen = n;
		} else {
			/* Use datagrams. */
			n = send_dg(statp, buf, buflen, ans, anssiz, &terrno,
				    ns, &v_circuit, &gotsomewhere);
			if (n < 0)
				return (-1);
			if (n == 0)
				goto next_ns;
			if (v_circuit)
				goto same_ns;
			resplen = n;
		}

		Dprint((statp->options & RES_DEBUG) ||
		       ((statp->pfcode & RES_PRF_REPLY) &&
			(statp->pfcode & RES_PRF_HEAD1)),
		       (stdout, ";; got answer:\n"));

		DprintQ((statp->options & RES_DEBUG) ||
			(statp->pfcode & RES_PRF_REPLY),
			(stdout, ""),
			ans, (resplen > anssiz) ? anssiz : resplen);

		/*
		 * If we have temporarily opened a virtual circuit,
		 * or if we haven't been asked to keep a socket open,
		 * close the socket.
		 */
		if ((v_circuit && (statp->options & RES_USEVC) == 0) ||
		    (statp->options & RES_STAYOPEN) == 0) {
			res_nclose(statp);
		}
		if (statp->rhook) {
			int done = 0, loops = 0;

			do {
				res_sendhookact act;

#ifdef _LIBC
				act = (*statp->rhook)((struct sockaddr_in *)
						      nsap, buf, buflen,
						      ans, anssiz, &resplen);
#else
				act = (*statp->rhook)(nsap, buf, buflen,
						      ans, anssiz, &resplen);
#endif
				switch (act) {
				case res_goahead:
				case res_done:
					done = 1;
					break;
				case res_nextns:
					res_nclose(statp);
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
		return (resplen);
 next_ns: ;
	   } /*foreach ns*/
	} /*foreach retry*/
	res_nclose(statp);
	if (!v_circuit) {
		if (!gotsomewhere)
			__set_errno (ECONNREFUSED);	/* no nameservers found */
		else
			__set_errno (ETIMEDOUT);	/* no answer obtained */
	} else
		__set_errno (terrno);
	return (-1);
}

/* Private */

static int
send_vc(res_state statp,
	const u_char *buf, int buflen, u_char *ans, int anssiz,
	int *terrno, int ns)
{
	const HEADER *hp = (HEADER *) buf;
	HEADER *anhp = (HEADER *) ans;
#ifdef _LIBC
	struct sockaddr_in6 *nsap = EXT(statp).nsaddrs[ns];
#else
	struct sockaddr_in *nsap = &statp->nsaddr_list[ns];
#endif
	int truncating, connreset, resplen, n;
	struct iovec iov[2];
	u_short len;
	u_char *cp;

	connreset = 0;
 same_ns:
	truncating = 0;

	/* Are we still talking to whom we want to talk to? */
	if (statp->_vcsock >= 0 && (statp->_flags & RES_F_VC) != 0) {
#ifdef _LIBC
		struct sockaddr_in6 peer;
#else
		struct sockaddr_in peer;
#endif
		int size = sizeof peer;

		if (getpeername(statp->_vcsock,
				(struct sockaddr *)&peer, &size) < 0 ||
		    !sock_eq(&peer, nsap)) {
			res_nclose(statp);
			statp->_flags &= ~RES_F_VC;
		}
	}

	if (statp->_vcsock < 0 || (statp->_flags & RES_F_VC) == 0) {
		if (statp->_vcsock >= 0)
			res_nclose(statp);

#ifdef _LIBC
		statp->_vcsock = socket(nsap->sin6_family, SOCK_STREAM, 0);
#else
		statp->_vcsock = socket(PF_INET, SOCK_STREAM, 0);
		if (statp->_vcsock > highestFD) {
			res_nclose(statp);
			__set_errno (ENOTSOCK);
		}
#endif
		if (statp->_vcsock < 0) {
			*terrno = errno;
			Perror(statp, stderr, "socket(vc)", errno);
			return (-1);
		}
		__set_errno (0);
		if (connect(statp->_vcsock, (struct sockaddr *)nsap,
			    sizeof *nsap) < 0) {
			*terrno = errno;
			Aerror(statp, stderr, "connect/vc", errno, *nsap);
			res_nclose(statp);
			return (0);
		}
		statp->_flags |= RES_F_VC;
	}

	/*
	 * Send length & message
	 */
	putshort((u_short)buflen, (u_char*)&len);
	iov[0] = evConsIovec(&len, INT16SZ);
	iov[1] = evConsIovec((void*)buf, buflen);
	if (writev(statp->_vcsock, iov, 2) != (INT16SZ + buflen)) {
		*terrno = errno;
		Perror(statp, stderr, "write failed", errno);
		res_nclose(statp);
		return (0);
	}
	/*
	 * Receive length & response
	 */
 read_len:
	cp = ans;
	len = INT16SZ;
	while ((n = read(statp->_vcsock, (char *)cp, (int)len)) > 0) {
		cp += n;
		if ((len -= n) <= 0)
			break;
	}
	if (n <= 0) {
		*terrno = errno;
		Perror(statp, stderr, "read failed", errno);
		res_nclose(statp);
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
			res_nclose(statp);
			goto same_ns;
		}
		res_nclose(statp);
		return (0);
	}
	resplen = ns_get16(ans);
	if (resplen > anssiz) {
		Dprint(statp->options & RES_DEBUG,
		       (stdout, ";; response truncated\n")
		       );
		truncating = 1;
		len = anssiz;
	} else
		len = resplen;
	if (len < HFIXEDSZ) {
		/*
		 * Undersized message.
		 */
		Dprint(statp->options & RES_DEBUG,
		       (stdout, ";; undersized: %d\n", len));
		*terrno = EMSGSIZE;
		res_nclose(statp);
		return (0);
	}
	cp = ans;
	while (len != 0 && (n = read(statp->_vcsock, (char *)cp, (int)len)) > 0){
		cp += n;
		len -= n;
	}
	if (n <= 0) {
		*terrno = errno;
		Perror(statp, stderr, "read(vc)", errno);
		res_nclose(statp);
		return (0);
	}
	if (truncating) {
		/*
		 * Flush rest of answer so connection stays in synch.
		 */
		anhp->tc = 1;
		len = resplen - anssiz;
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
	if (hp->id != anhp->id) {
		DprintQ((statp->options & RES_DEBUG) ||
			(statp->pfcode & RES_PRF_REPLY),
			(stdout, ";; old answer (unexpected):\n"),
			ans, (resplen > anssiz) ? anssiz: resplen);
		goto read_len;
	}

	/*
	 * All is well, or the error is fatal.  Signal that the
	 * next nameserver ought not be tried.
	 */
	return (resplen);
}

static int
send_dg(res_state statp,
	const u_char *buf, int buflen, u_char *ans, int anssiz,
	int *terrno, int ns, int *v_circuit, int *gotsomewhere)
{
	const HEADER *hp = (HEADER *) buf;
	HEADER *anhp = (HEADER *) ans;
#ifdef _LIBC
	struct sockaddr_in6 *nsap = EXT(statp).nsaddrs[ns];
#else
	const struct sockaddr_in *nsap = &statp->nsaddr_list[ns];
#endif
	struct timespec now, timeout, finish;
#ifdef _LIBC
	struct pollfd pfd[1];
        int ptimeout;
	struct sockaddr_in6 from;
	static int socket_pf = 0;
#else
	fd_set dsmask;
	struct sockaddr_in from;
#endif
	int fromlen, resplen, seconds, n, s;

	if (EXT(statp).nssocks[ns] == -1) {
#ifdef _LIBC
		/* only try IPv6 if IPv6 NS and if not failed before */
		if ((EXT(statp).nscount6 > 0) && (socket_pf != PF_INET)) {
			EXT(statp).nssocks[ns] =
			    socket(PF_INET6, SOCK_DGRAM, 0);
			socket_pf = EXT(statp).nssocks[ns] < 0 ? PF_INET
			                                       : PF_INET6;
		}
		if (EXT(statp).nssocks[ns] < 0)
			EXT(statp).nssocks[ns] = socket(PF_INET, SOCK_DGRAM, 0);
#else
		EXT(statp).nssocks[ns] = socket(PF_INET, SOCK_DGRAM, 0);
		if (EXT(statp).nssocks[ns] > highestFD) {
			res_nclose(statp);
			__set_errno (ENOTSOCK);
		}
#endif
		if (EXT(statp).nssocks[ns] < 0) {
			*terrno = errno;
			Perror(statp, stderr, "socket(dg)", errno);
			return (-1);
		}
#ifndef CANNOT_CONNECT_DGRAM
#ifdef _LIBC
		/* If IPv6 socket and nsap is IPv4, make it IPv4-mapped */
		if ((socket_pf == PF_INET6) && (nsap->sin6_family == AF_INET))
			convaddr4to6(nsap);
#endif
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
		if (connect(EXT(statp).nssocks[ns], (struct sockaddr *)nsap,
			    sizeof *nsap) < 0) {
			Aerror(statp, stderr, "connect(dg)", errno, *nsap);
			res_nclose(statp);
			return (0);
		}
#endif /* !CANNOT_CONNECT_DGRAM */
		Dprint(statp->options & RES_DEBUG,
		       (stdout, ";; new DG socket\n"))
	}
	s = EXT(statp).nssocks[ns];
#ifndef CANNOT_CONNECT_DGRAM
	if (send(s, (char*)buf, buflen, 0) != buflen) {
		Perror(statp, stderr, "send", errno);
		res_nclose(statp);
		return (0);
	}
#else /* !CANNOT_CONNECT_DGRAM */
#ifdef _LIBC
	/* If IPv6 socket and nsap is IPv4, make it IPv4-mapped */
	if ((socket_pf == PF_INET6) && (nsap->sin6_family == AF_INET))
		convaddr4to6(nsap);
#endif
	if (sendto(s, (char*)buf, buflen, 0,
		   (struct sockaddr *)nsap, sizeof *nsap) != buflen)
	{
		Aerror(statp, stderr, "sendto", errno, *nsap);
		res_nclose(statp);
		return (0);
	}
#endif /* !CANNOT_CONNECT_DGRAM */

	/*
	 * Wait for reply.
	 */
	seconds = (statp->retrans << ns);
	if (ns > 0)
		seconds /= statp->nscount;
	if (seconds <= 0)
		seconds = 1;
	now = evNowTime();
	timeout = evConsTime(seconds, 0);
	finish = evAddTime(now, timeout);
 wait:
#ifdef _LIBC
        /* Convert struct timespec in milliseconds.  */
	ptimeout = timeout.tv_sec * 1000 + timeout.tv_nsec / 1000000;

	pfd[0].fd = s;
	pfd[0].events = POLLIN;
	n = __poll (pfd, 1, ptimeout);
#else
	FD_ZERO(&dsmask);
	FD_SET(s, &dsmask);
	n = pselect(s + 1, &dsmask, NULL, NULL, &timeout, NULL);
#endif
	if (n == 0) {
		Dprint(statp->options & RES_DEBUG, (stdout, ";; timeout\n"));
		*gotsomewhere = 1;
		return (0);
	}
	if (n < 0) {
		if (errno == EINTR) {
			now = evNowTime();
			if (evCmpTime(finish, now) > 0) {
				timeout = evSubTime(finish, now);
				goto wait;
			}
		}
		Perror(statp, stderr, "select", errno);
		res_nclose(statp);
		return (0);
	}
	__set_errno (0);
#ifdef _LIBC
	fromlen = sizeof(struct sockaddr_in6);
#else
	fromlen = sizeof(struct sockaddr_in);
#endif
	resplen = recvfrom(s, (char*)ans, anssiz,0,
			   (struct sockaddr *)&from, &fromlen);
	if (resplen <= 0) {
		Perror(statp, stderr, "recvfrom", errno);
		res_nclose(statp);
		return (0);
	}
	*gotsomewhere = 1;
	if (resplen < HFIXEDSZ) {
		/*
		 * Undersized message.
		 */
		Dprint(statp->options & RES_DEBUG,
		       (stdout, ";; undersized: %d\n",
			resplen));
		*terrno = EMSGSIZE;
		res_nclose(statp);
		return (0);
	}
	if (hp->id != anhp->id) {
		/*
		 * response from old query, ignore it.
		 * XXX - potential security hazard could
		 *	 be detected here.
		 */
		DprintQ((statp->options & RES_DEBUG) ||
			(statp->pfcode & RES_PRF_REPLY),
			(stdout, ";; old answer:\n"),
			ans, (resplen > anssiz) ? anssiz : resplen);
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
			ans, (resplen > anssiz) ? anssiz : resplen);
		goto wait;
	}
	if (!(statp->options & RES_INSECURE2) &&
	    !res_queriesmatch(buf, buf + buflen,
			      ans, ans + anssiz)) {
		/*
		 * response contains wrong query? ignore it.
		 * XXX - potential security hazard could
		 *	 be detected here.
		 */
		DprintQ((statp->options & RES_DEBUG) ||
			(statp->pfcode & RES_PRF_REPLY),
			(stdout, ";; wrong query name:\n"),
			ans, (resplen > anssiz) ? anssiz : resplen);
		goto wait;
	}
	if (anhp->rcode == SERVFAIL ||
	    anhp->rcode == NOTIMP ||
	    anhp->rcode == REFUSED) {
		DprintQ(statp->options & RES_DEBUG,
			(stdout, "server rejected query:\n"),
			ans, (resplen > anssiz) ? anssiz : resplen);
		res_nclose(statp);
		/* don't retry if called from dig */
		if (!statp->pfcode)
			return (0);
	}
	if (!(statp->options & RES_IGNTC) && anhp->tc) {
		/*
		 * To get the rest of answer,
		 * use TCP with same server.
		 */
		Dprint(statp->options & RES_DEBUG,
		       (stdout, ";; truncated answer\n"));
		*v_circuit = 1;
		res_nclose(statp);
		return (1);
	}
	/*
	 * All is well, or the error is fatal.  Signal that the
	 * next nameserver ought not be tried.
	 */
	return (resplen);
}

#ifdef DEBUG
static void
Aerror(const res_state statp, FILE *file, const char *string, int error,
       struct sockaddr_in address)
{
	int save = errno;

	if ((statp->options & RES_DEBUG) != 0) {
		char tmp[sizeof "255.255.255.255"];

		fprintf(file, "res_send: %s ([%s].%u): %s\n",
			string,
			inet_ntop(address.sin_family, &address.sin_addr,
				  tmp, sizeof tmp),
			ntohs(address.sin_port),
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
#ifdef _LIBC
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
#else
sock_eq(struct sockaddr_in *a1, struct sockaddr_in *a2) {
	return ((a1->sin_family == a2->sin_family) &&
		(a1->sin_port == a2->sin_port) &&
		(a1->sin_addr.s_addr == a2->sin_addr.s_addr));
}
#endif

#ifdef _LIBC
/*
 * Converts IPv4 family, address and port to
 * IPv6 family, IPv4-mapped IPv6 address and port.
 */
static void
convaddr4to6(struct sockaddr_in6 *sa)
{
    struct sockaddr_in *sa4p = (struct sockaddr_in *) sa;
    in_port_t port = sa4p->sin_port;
    in_addr_t addr = sa4p->sin_addr.s_addr;

    sa->sin6_family = AF_INET6;
    sa->sin6_port = port;
    sa->sin6_addr.s6_addr32[0] = 0;
    sa->sin6_addr.s6_addr32[1] = 0;
    sa->sin6_addr.s6_addr32[2] = htonl(0xFFFF);
    sa->sin6_addr.s6_addr32[3] = addr;
}
#endif

#ifdef NEED_PSELECT
/* XXX needs to move to the porting library. */
static int
pselect(int nfds, void *rfds, void *wfds, void *efds,
	struct timespec *tsp, const sigset_t *sigmask)
{
	struct timeval tv, *tvp;
	sigset_t sigs;
	int n;

	if (tsp) {
		tvp = &tv;
		tv = evTimeVal(*tsp);
	} else
		tvp = NULL;
	if (sigmask)
		sigprocmask(SIG_SETMASK, sigmask, &sigs);
	n = select(nfds, rfds, wfds, efds, tvp);
	if (sigmask)
		sigprocmask(SIG_SETMASK, &sigs, NULL);
	if (tsp)
		*tsp = evTimeSpec(tv);
	return (n);
}
#endif
