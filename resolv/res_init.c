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

#include <ctype.h>
#include <netdb.h>
#include <resolv.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <inet/net-internal.h>

#include <not-cancel.h>

/* Options.  Should all be left alone. */
/* #undef DEBUG */

static void res_setoptions (res_state, const char *, const char *)
     internal_function;

static const char sort_mask_chars[] = "/&";
#define ISSORTMASK(ch) (strchr(sort_mask_chars, ch) != NULL)
static u_int32_t net_mask (struct in_addr) __THROW;

unsigned long long int __res_initstamp attribute_hidden;

/*
 * Resolver state default settings.
 */

/*
 * Set up default settings.  If the configuration file exist, the values
 * there will have precedence.  Otherwise, the server address is set to
 * INADDR_LOOPBACK and the default domain name comes from gethostname.
 *
 * Return 0 if completes successfully, -1 on error
 */
int
res_ninit(res_state statp) {
	extern int __res_vinit(res_state, int);

	return (__res_vinit(statp, 0));
}
libc_hidden_def (__res_ninit)

/* This function has to be reachable by res_data.c but not publically. */
int
__res_vinit(res_state statp, int preinit) {
	FILE *fp;
	char *cp, **pp;
	int n;
	char buf[BUFSIZ];
	int nserv = 0;    /* number of nameservers read from file */
	int have_serv6 = 0;
	int haveenv = 0;
	int havesearch = 0;
	int nsort = 0;
	char *net;
	statp->_u._ext.initstamp = __res_initstamp;

	if (!preinit) {
		statp->retrans = RES_TIMEOUT;
		statp->retry = RES_DFLRETRY;
		statp->options = RES_DEFAULT;
		statp->id = res_randomid();
	}

	statp->nscount = 0;
	statp->defdname[0] = '\0';
	statp->ndots = 1;
	statp->pfcode = 0;
	statp->_vcsock = -1;
	statp->_flags = 0;
	statp->__glibc_unused_qhook = NULL;
	statp->__glibc_unused_rhook = NULL;
	statp->_u._ext.nscount = 0;
	for (n = 0; n < MAXNS; n++)
	    statp->_u._ext.nsaddrs[n] = NULL;

	/* Allow user to override the local domain definition */
	if ((cp = getenv("LOCALDOMAIN")) != NULL) {
		(void)strncpy(statp->defdname, cp, sizeof(statp->defdname) - 1);
		statp->defdname[sizeof(statp->defdname) - 1] = '\0';
		haveenv++;

		/*
		 * Set search list to be blank-separated strings
		 * from rest of env value.  Permits users of LOCALDOMAIN
		 * to still have a search list, and anyone to set the
		 * one that they want to use as an individual (even more
		 * important now that the rfc1535 stuff restricts searches)
		 */
		cp = statp->defdname;
		pp = statp->dnsrch;
		*pp++ = cp;
		for (n = 0; *cp && pp < statp->dnsrch + MAXDNSRCH; cp++) {
			if (*cp == '\n')	/* silly backwards compat */
				break;
			else if (*cp == ' ' || *cp == '\t') {
				*cp = 0;
				n = 1;
			} else if (n) {
				*pp++ = cp;
				n = 0;
				havesearch = 1;
			}
		}
		/* null terminate last domain if there are excess */
		while (*cp != '\0' && *cp != ' ' && *cp != '\t' && *cp != '\n')
			cp++;
		*cp = '\0';
		*pp++ = 0;
	}

#define	MATCH(line, name) \
	(!strncmp(line, name, sizeof(name) - 1) && \
	(line[sizeof(name) - 1] == ' ' || \
	 line[sizeof(name) - 1] == '\t'))

	if ((fp = fopen(_PATH_RESCONF, "rce")) != NULL) {
	    /* No threads use this stream.  */
	    __fsetlocking (fp, FSETLOCKING_BYCALLER);
	    /* read the config file */
	    while (__fgets_unlocked(buf, sizeof(buf), fp) != NULL) {
		/* skip comments */
		if (*buf == ';' || *buf == '#')
			continue;
		/* read default domain name */
		if (MATCH(buf, "domain")) {
		    if (haveenv)	/* skip if have from environ */
			    continue;
		    cp = buf + sizeof("domain") - 1;
		    while (*cp == ' ' || *cp == '\t')
			    cp++;
		    if ((*cp == '\0') || (*cp == '\n'))
			    continue;
		    strncpy(statp->defdname, cp, sizeof(statp->defdname) - 1);
		    statp->defdname[sizeof(statp->defdname) - 1] = '\0';
		    if ((cp = strpbrk(statp->defdname, " \t\n")) != NULL)
			    *cp = '\0';
		    havesearch = 0;
		    continue;
		}
		/* set search list */
		if (MATCH(buf, "search")) {
		    if (haveenv)	/* skip if have from environ */
			    continue;
		    cp = buf + sizeof("search") - 1;
		    while (*cp == ' ' || *cp == '\t')
			    cp++;
		    if ((*cp == '\0') || (*cp == '\n'))
			    continue;
		    strncpy(statp->defdname, cp, sizeof(statp->defdname) - 1);
		    statp->defdname[sizeof(statp->defdname) - 1] = '\0';
		    if ((cp = strchr(statp->defdname, '\n')) != NULL)
			    *cp = '\0';
		    /*
		     * Set search list to be blank-separated strings
		     * on rest of line.
		     */
		    cp = statp->defdname;
		    pp = statp->dnsrch;
		    *pp++ = cp;
		    for (n = 0; *cp && pp < statp->dnsrch + MAXDNSRCH; cp++) {
			    if (*cp == ' ' || *cp == '\t') {
				    *cp = 0;
				    n = 1;
			    } else if (n) {
				    *pp++ = cp;
				    n = 0;
			    }
		    }
		    /* null terminate last domain if there are excess */
		    while (*cp != '\0' && *cp != ' ' && *cp != '\t')
			    cp++;
		    *cp = '\0';
		    *pp++ = 0;
		    havesearch = 1;
		    continue;
		}
		/* read nameservers to query */
		if (MATCH(buf, "nameserver") && nserv < MAXNS) {
		    struct in_addr a;

		    cp = buf + sizeof("nameserver") - 1;
		    while (*cp == ' ' || *cp == '\t')
			cp++;
		    if ((*cp != '\0') && (*cp != '\n')
			&& __inet_aton(cp, &a)) {
			statp->nsaddr_list[nserv].sin_addr = a;
			statp->nsaddr_list[nserv].sin_family = AF_INET;
			statp->nsaddr_list[nserv].sin_port =
				htons(NAMESERVER_PORT);
			nserv++;
		    } else {
			struct in6_addr a6;
			char *el;

			if ((el = strpbrk(cp, " \t\n")) != NULL)
			    *el = '\0';
			if ((el = strchr(cp, SCOPE_DELIMITER)) != NULL)
			    *el = '\0';
			if ((*cp != '\0') &&
			    (__inet_pton(AF_INET6, cp, &a6) > 0)) {
			    struct sockaddr_in6 *sa6;

			    sa6 = malloc(sizeof(*sa6));
			    if (sa6 != NULL) {
				sa6->sin6_family = AF_INET6;
				sa6->sin6_port = htons(NAMESERVER_PORT);
				sa6->sin6_flowinfo = 0;
				sa6->sin6_addr = a6;

				sa6->sin6_scope_id = 0;
				if (__glibc_likely (el != NULL)) {
				  /* Ignore errors, for backwards
				     compatibility.  */
				  (void) __inet6_scopeid_pton
				    (&a6, el + 1, &sa6->sin6_scope_id);
				}

				statp->nsaddr_list[nserv].sin_family = 0;
				statp->_u._ext.nsaddrs[nserv] = sa6;
				statp->_u._ext.nssocks[nserv] = -1;
				have_serv6 = 1;
				nserv++;
			    }
			}
		    }
		    continue;
		}
		if (MATCH(buf, "sortlist")) {
		    struct in_addr a;

		    cp = buf + sizeof("sortlist") - 1;
		    while (nsort < MAXRESOLVSORT) {
			while (*cp == ' ' || *cp == '\t')
			    cp++;
			if (*cp == '\0' || *cp == '\n' || *cp == ';')
			    break;
			net = cp;
			while (*cp && !ISSORTMASK(*cp) && *cp != ';' &&
			       isascii(*cp) && !isspace(*cp))
				cp++;
			n = *cp;
			*cp = 0;
			if (__inet_aton(net, &a)) {
			    statp->sort_list[nsort].addr = a;
			    if (ISSORTMASK(n)) {
				*cp++ = n;
				net = cp;
				while (*cp && *cp != ';' &&
					isascii(*cp) && !isspace(*cp))
				    cp++;
				n = *cp;
				*cp = 0;
				if (__inet_aton(net, &a)) {
				    statp->sort_list[nsort].mask = a.s_addr;
				} else {
				    statp->sort_list[nsort].mask =
					net_mask(statp->sort_list[nsort].addr);
				}
			    } else {
				statp->sort_list[nsort].mask =
				    net_mask(statp->sort_list[nsort].addr);
			    }
			    nsort++;
			}
			*cp = n;
		    }
		    continue;
		}
		if (MATCH(buf, "options")) {
		    res_setoptions(statp, buf + sizeof("options") - 1, "conf");
		    continue;
		}
	    }
	    statp->nscount = nserv;
	    if (have_serv6) {
		/* We try IPv6 servers again.  */
		statp->ipv6_unavail = false;
	    }
	    statp->nsort = nsort;
	    (void) fclose(fp);
	}
	if (__builtin_expect(statp->nscount == 0, 0)) {
	    statp->nsaddr.sin_addr = __inet_makeaddr(IN_LOOPBACKNET, 1);
	    statp->nsaddr.sin_family = AF_INET;
	    statp->nsaddr.sin_port = htons(NAMESERVER_PORT);
	    statp->nscount = 1;
	}
	if (statp->defdname[0] == 0 &&
	    __gethostname(buf, sizeof(statp->defdname) - 1) == 0 &&
	    (cp = strchr(buf, '.')) != NULL)
		strcpy(statp->defdname, cp + 1);

	/* find components of local domain that might be searched */
	if (havesearch == 0) {
		pp = statp->dnsrch;
		*pp++ = statp->defdname;
		*pp = NULL;

	}

	if ((cp = getenv("RES_OPTIONS")) != NULL)
		res_setoptions(statp, cp, "env");
	statp->options |= RES_INIT;
	return (0);
}

static void
internal_function
res_setoptions(res_state statp, const char *options, const char *source) {
	const char *cp = options;
	int i;

#ifdef DEBUG
	if (statp->options & RES_DEBUG)
		printf(";; res_setoptions(\"%s\", \"%s\")...\n",
		       options, source);
#endif
	while (*cp) {
		/* skip leading and inner runs of spaces */
		while (*cp == ' ' || *cp == '\t')
			cp++;
		/* search for and process individual options */
		if (!strncmp(cp, "ndots:", sizeof("ndots:") - 1)) {
			i = atoi(cp + sizeof("ndots:") - 1);
			if (i <= RES_MAXNDOTS)
				statp->ndots = i;
			else
				statp->ndots = RES_MAXNDOTS;
#ifdef DEBUG
			if (statp->options & RES_DEBUG)
				printf(";;\tndots=%d\n", statp->ndots);
#endif
		} else if (!strncmp(cp, "timeout:", sizeof("timeout:") - 1)) {
			i = atoi(cp + sizeof("timeout:") - 1);
			if (i <= RES_MAXRETRANS)
				statp->retrans = i;
			else
				statp->retrans = RES_MAXRETRANS;
		} else if (!strncmp(cp, "attempts:", sizeof("attempts:") - 1)){
			i = atoi(cp + sizeof("attempts:") - 1);
			if (i <= RES_MAXRETRY)
				statp->retry = i;
			else
				statp->retry = RES_MAXRETRY;
		} else if (!strncmp(cp, "debug", sizeof("debug") - 1)) {
#ifdef DEBUG
			if (!(statp->options & RES_DEBUG)) {
				printf(";; res_setoptions(\"%s\", \"%s\")..\n",
				       options, source);
				statp->options |= RES_DEBUG;
			}
			printf(";;\tdebug\n");
#endif
		} else {
		  static const struct
		  {
		    char str[22];
		    uint8_t len;
		    uint8_t clear;
		    unsigned long int flag;
		  } options[] = {
#define STRnLEN(str) str, sizeof (str) - 1
		    { STRnLEN ("inet6"), 0, RES_USE_INET6 },
		    { STRnLEN ("rotate"), 0, RES_ROTATE },
		    { STRnLEN ("edns0"), 0, RES_USE_EDNS0 },
		    { STRnLEN ("single-request-reopen"), 0, RES_SNGLKUPREOP },
		    { STRnLEN ("single-request"), 0, RES_SNGLKUP },
		    { STRnLEN ("no_tld_query"), 0, RES_NOTLDQUERY },
		    { STRnLEN ("no-tld-query"), 0, RES_NOTLDQUERY },
		    { STRnLEN ("use-vc"), 0, RES_USEVC }
		  };
#define noptions (sizeof (options) / sizeof (options[0]))
		  int i;
		  for (i = 0; i < noptions; ++i)
		    if (strncmp (cp, options[i].str, options[i].len) == 0)
		      {
			if (options[i].clear)
			  statp->options &= options[i].flag;
			else
			  statp->options |= options[i].flag;
			break;
		      }
		  if (i == noptions) {
		    /* XXX - print a warning here? */
		  }
		}
		/* skip to next run of spaces */
		while (*cp && *cp != ' ' && *cp != '\t')
			cp++;
	}
}

/* XXX - should really support CIDR which means explicit masks always. */
/* XXX - should really use system's version of this */
static u_int32_t
net_mask (struct in_addr in)
{
	u_int32_t i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (htonl(IN_CLASSA_NET));
	else if (IN_CLASSB(i))
		return (htonl(IN_CLASSB_NET));
	return (htonl(IN_CLASSC_NET));
}

u_int
res_randomid(void) {
	return 0xffff & __getpid();
}
libc_hidden_def (__res_randomid)


/*
 * This routine is for closing the socket if a virtual circuit is used and
 * the program wants to close it.  This provides support for endhostent()
 * which expects to close the socket.
 *
 * This routine is not expected to be user visible.
 */
void
__res_iclose(res_state statp, bool free_addr) {
	int ns;

	if (statp->_vcsock >= 0) {
		close_not_cancel_no_status(statp->_vcsock);
		statp->_vcsock = -1;
		statp->_flags &= ~(RES_F_VC | RES_F_CONN);
	}
	for (ns = 0; ns < statp->nscount; ns++)
		if (statp->_u._ext.nsaddrs[ns]) {
			if (statp->_u._ext.nssocks[ns] != -1) {
				close_not_cancel_no_status(statp->_u._ext.nssocks[ns]);
				statp->_u._ext.nssocks[ns] = -1;
			}
			if (free_addr) {
				free (statp->_u._ext.nsaddrs[ns]);
				statp->_u._ext.nsaddrs[ns] = NULL;
			}
		}
}
libc_hidden_def (__res_iclose)

void
res_nclose(res_state statp)
{
  __res_iclose (statp, true);
}
libc_hidden_def (__res_nclose)

/* This is called when a thread is exiting to free resources held in _res.  */
static void __attribute__ ((section ("__libc_thread_freeres_fn")))
res_thread_freeres (void)
{
  if (_res.nscount == 0)
    /* Never called res_ninit.  */
    return;

  __res_iclose (&_res, true);		/* Close any VC sockets.  */

  /* Make sure we do a full re-initialization the next time.  */
  _res.options = 0;
}
text_set_element (__libc_thread_subfreeres, res_thread_freeres);
text_set_element (__libc_subfreeres, res_thread_freeres);
