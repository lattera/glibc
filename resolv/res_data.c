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

#include <resolv.h>

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
