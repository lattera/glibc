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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

/* This file was broken out from:
	$Log$
	Revision 1.2  1997/03/16 17:41:36  drepper
	(__mach_error_map_compat): Give full prototype.

	Revision 1.2  1997/03/14 15:26:28  thomas
	Wed Mar  5 10:40:05 1997  Thomas Bushnell, n/BSG  <thomas@gnu.ai.mit.edu>

		* mach/mach_error.c (mach_error_string_int): Give full prototype.
		* mach/errstring.c (mach_error_string_int): Likewise.
		* mach/error_compat.c (__mach_error_map_compat): Likewise.

		* mach/spin-solid.c: Include <mach/mach_traps.h>.
		* mach/spin-solid.c (__spin_lock_solid): Provide arg to
		swtch_pri.

		* mach/mach_init.c: Include <mach/mig_support.h>.

		* mach/mach_error.h (mach_error_string, mach_error,
		mach_error_type): Always provide prototypes.

	Revision 1.1  1993/11/30 17:35:24  roland
	entered into RCS

	Revision 2.3  92/04/01  19:38:18  rpd
   The static do_compat function is renamed to be globally accessible.
 */

#include <mach/error.h>
#include <mach_error.h>
#include <errorlib.h>


void
__mach_error_map_compat(mach_error_t  *org_err)
{
	mach_error_t		err = *org_err;

	/* 
	 * map old error numbers to 
	 * to new error sys & subsystem 
	 */

	if ((-200 < err) && (err <= -100))
		err = -(err + 100) | IPC_SEND_MOD;
	else if ((-300 < err) && (err <= -200))
		err = -(err + 200) | IPC_RCV_MOD;
	else if ((-400 < err) && (err <= -300))
		err = -(err + 300) | MACH_IPC_MIG_MOD;
	else if ((1000 <= err) && (err < 1100))
		err = (err - 1000) | SERV_NETNAME_MOD;
	else if ((1600 <= err) && (err < 1700))
		err = (err - 1600) | SERV_ENV_MOD;
	else if ((27600 <= err) && (err < 27700))
		err = (err - 27600) | SERV_EXECD_MOD;
	else if ((2500 <= err) && (err < 2600))
		err = (err - 2500) | KERN_DEVICE_MOD;
	else if ((5000 <= err) && (err < 5100))
		err = (err - 5000) | BOOTSTRAP_FS_MOD;

	*org_err = err;
}
