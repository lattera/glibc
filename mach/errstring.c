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
/*
 * HISTORY
 * $Log$
 * Revision 1.2  1997/03/16 17:41:48  drepper
 * (mach_error_string_int): Give full prototype.
 *
 * Revision 1.2  1997/03/14 15:26:29  thomas
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
 * Revision 1.1  1993/11/30 17:35:58  roland
 * entered into RCS
 *
 * Revision 2.3  92/04/01  19:38:18  rpd
 * 	Updated do_compat for kernel device errors,
 * 	bootstrap file-system errors.
 * 	[92/03/09            rpd]
 * 
 * Revision 2.2  92/02/20  15:58:08  elf
 * 	Created from mach_error.c.
 * 	[92/02/11            rpd]
 * 
 */

#define EXPORT_BOOLEAN
#include <mach/boolean.h>
#include <mach/error.h>
#include <mach_error.h>
#include <errorlib.h>

extern void __mach_error_map_compat (mach_error_t *);

const char *
mach_error_type( err )
	mach_error_t		err;
{
	int sub, system;

	__mach_error_map_compat( &err );

	sub = err_get_sub(err);
	system = err_get_system(err);

	if (system > err_max_system
	||  sub >= errors[system].max_sub ) return( "(?/?)" );
	return( errors[system].subsystem[sub].subsys_name );
}

boolean_t mach_error_full_diag = FALSE;

const char *
mach_error_string_int(mach_error_t	err,
		      boolean_t		* diag)
{
	int sub, system, code;

	__mach_error_map_compat( &err );

	sub = err_get_sub(err);
	system = err_get_system(err);
	code = err_get_code(err);

	*diag = TRUE;

	if (system > err_max_system) return( "(?/?) unknown error system" );
	if (sub >= errors[system].max_sub) return( errors[system].bad_sub );
	if (code >= errors[system].subsystem[sub].max_code) return ( NO_SUCH_ERROR );

	*diag = mach_error_full_diag;
	return( errors[system].subsystem[sub].codes[code] );
}

const char *
mach_error_string( err )
	mach_error_t		err;
{
	boolean_t diag;

	return mach_error_string_int( err, &diag );

}
