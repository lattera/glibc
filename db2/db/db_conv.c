/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995
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

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_conv.c	10.5 (Sleepycat) 9/3/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_swap.h"
#include "db_am.h"

static int __db_convert __P((db_pgno_t, void *, int));

/*
 * __db_pgin, __db_pgout --
 *
 * PUBLIC: int __db_pgin __P((db_pgno_t, void *));
 * PUBLIC: int __db_pgout __P((db_pgno_t, void *));
 */
int
__db_pgin(pg, pp)
	db_pgno_t pg;
	void *pp;
{
	return (__db_convert(pg, pp, 1));
}

int
__db_pgout(pg, pp)
	db_pgno_t pg;
	void *pp;
{
	return (__db_convert(pg, pp, 0));
}

/*
 * __db_convert --
 *	Actually convert a page.
 */
static int
__db_convert(pg, pp, pgin)
	db_pgno_t pg;			/* Unused, but left for the future. */
	void *pp;
	int pgin;
{
	BINTERNAL *bi;
	BKEYDATA *bk;
	BOVERFLOW *bo;
	HKEYDATA *hk;
	PAGE *h;
	RINTERNAL *ri;
	db_indx_t i;
	u_int8_t *p;

	h = pp;
	if (pgin) {
		M_32_SWAP(h->lsn.file);
		M_32_SWAP(h->lsn.offset);
		M_32_SWAP(h->pgno);
		M_32_SWAP(h->prev_pgno);
		M_32_SWAP(h->next_pgno);
		M_16_SWAP(h->entries);
		M_16_SWAP(h->hf_offset);
	}

	switch (h->type) {
	case P_HASH:
		for (i = 0; i < NUM_ENT(h); i++) {
			if (pgin)
				M_16_SWAP(h->inp[i]);

			hk = GET_HKEYDATA(h, i);
			switch (hk->type) {
			case H_KEYDATA:
				break;
			case H_DUPLICATE:
			case H_OFFPAGE:
				p = (u_int8_t *)hk + sizeof(u_int8_t);
				++p;
				SWAP32(p);			/* tlen */
				SWAP32(p);			/* pgno */
				SWAP16(p);			/* offset */
				SWAP16(p);			/* len */
				break;
			}

			if (!pgin)
				M_16_SWAP(h->inp[i]);
		}
		break;
	case P_LBTREE:
	case P_LRECNO:
	case P_DUPLICATE:
		for (i = 0; i < NUM_ENT(h); i++) {
			if (pgin)
				M_16_SWAP(h->inp[i]);

			bk = GET_BKEYDATA(h, i);
			switch (B_TYPE(bk->type)) {
			case B_KEYDATA:
				M_16_SWAP(bk->len);
				break;
			case B_DUPLICATE:
			case B_OVERFLOW:
				bo = (BOVERFLOW *)bk;
				M_32_SWAP(bo->tlen);
				M_32_SWAP(bo->pgno);
				break;
			}

			if (!pgin)
				M_16_SWAP(h->inp[i]);
		}
		break;
	case P_IBTREE:
		for (i = 0; i < NUM_ENT(h); i++) {
			if (pgin)
				M_16_SWAP(h->inp[i]);

			bi = GET_BINTERNAL(h, i);
			switch (B_TYPE(bi->type)) {
			case B_KEYDATA:
				M_16_SWAP(bi->len);
				M_32_SWAP(bi->pgno);
				M_32_SWAP(bi->nrecs);
				break;
			case B_DUPLICATE:
			case B_OVERFLOW:
				bo = (BOVERFLOW *)bi;
				M_32_SWAP(bo->tlen);
				M_32_SWAP(bo->pgno);
				break;
			}

			if (!pgin)
				M_16_SWAP(h->inp[i]);
		}
		break;
	case P_IRECNO:
		for (i = 0; i < NUM_ENT(h); i++) {
			if (pgin)
				M_16_SWAP(h->inp[i]);

			ri = GET_RINTERNAL(h, i);
			M_32_SWAP(ri->pgno);
			M_32_SWAP(ri->nrecs);

			if (!pgin)
				M_16_SWAP(h->inp[i]);
		}
	case P_OVERFLOW:
	case P_INVALID:
		/* Nothing to do. */
		break;
	default:
		return (EINVAL);
	}

	if (!pgin) {
		/* Swap the header information. */
		M_32_SWAP(h->lsn.file);
		M_32_SWAP(h->lsn.offset);
		M_32_SWAP(h->pgno);
		M_32_SWAP(h->prev_pgno);
		M_32_SWAP(h->next_pgno);
		M_16_SWAP(h->entries);
		M_16_SWAP(h->hf_offset);
	}
	return (0);
}
