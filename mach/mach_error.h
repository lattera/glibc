/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log$
 * Revision 1.4  2001/04/01 05:03:14  roland
 * 2001-03-11  Roland McGrath  <roland@frob.com>
 *
 * 	* mach/mach_error.h: Fix ancient #endif syntax.
 * 	* hurd/hurdmalloc.c: Likewise.
 *
 * Revision 1.3  1997/03/16 17:42:25  drepper
 * (mach_error_string, mach_error, mach_error_type): Always provide
 * prototypes.
 * (mach_error_fn_t): Comment out declaration; it appears to be entirely
 * unused dead code.
 *
 * Revision 1.3  1997/03/14 15:26:31  thomas
 * Wed Mar  5 10:40:05 1997  Thomas Bushnell, n/BSG  <thomas@gnu.ai.mit.edu>
 *
 * 	* mach/mach_error.c (mach_error_string_int): Give full prototype.
 * 	* mach/errstring.c (mach_error_string_int): Likewise.
 * 	* mach/error_compat.c (__mach_error_map_compat): Likewise.
 *
 * 	* mach/spin-solid.c: Include <mach/mach_traps.h>.
 * 	* mach/spin-solid.c (__spin_lock_solid): Provide arg to
 * 	swtch_pri.
 *
 * 	* mach/mach_init.c: Include <mach/mig_support.h>.
 *
 * 	* mach/mach_error.h (mach_error_string, mach_error,
 * 	mach_error_type): Always provide prototypes.
 *
 * Revision 1.2  1993/11/23 20:39:08  mib
 * entered into RCS
 *
 * Revision 2.2  92/01/16  00:08:10  rpd
 * 	Moved from user collection to mk collection.
 *
 * Revision 2.2  91/03/27  15:39:13  mrt
 * 	First checkin
 *
 */
/*
 *	File:	mach_error.h
 *	Author:	Douglas Orr, Carnegie Mellon University
 *	Date:	Mar. 1988
 *
 *	Definitions of routines in mach_error.c
 */

#ifndef	_MACH_ERROR_
#define	_MACH_ERROR_	1

#include <mach/error.h>

const char	*mach_error_string(
/*
 *	Returns a string appropriate to the error argument given
 */
	mach_error_t error_value
				);

void		mach_error(
/*
 *	Prints an appropriate message on the standard error stream
 */
	char 		*str,
	mach_error_t	error_value
				);

const char	*mach_error_type(
/*
 *	Returns a string with the error system, subsystem and code
*/
	mach_error_t	error_value
				);

#endif	/* _MACH_ERROR_ */
