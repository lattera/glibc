/*
 *	nis_cache.x
 *
 *	Copyright (c) 1988-1992 Sun Microsystems Inc
 *	All Rights Reserved.
 */

%#pragma ident	"@(#)nis_cache.x	1.8	92/07/14 SMI"


#ifdef RPC_HDR
%#include <rpc/types.h>
%#include <rpcsvc/nis.h>
%
%/* default cache file */
%#define CACHEFILE "/var/nis/NIS_SHARED_DIRCACHE" 
%
%/* clients have to read-lock the cache file, and SVR4 locking requires that */
%/*   the file be writable, but we don't want a world-writable cache file.   */
%/*   So... everyone agrees to use a different, world-writable file for the  */
%/*   locking operations, but the data is in CACHEFILE.                      */
%#define CACHELOCK "/usr/tmp/.NIS_DIR_CACHELOCK"
%
%/* the file containing one trusted XDR'ed directory object.
% * This has to be present for the system to work.
% */
%#define COLD_START_FILE "/var/nis/NIS_COLD_START"
%
%enum pc_status {HIT, MISS, NEAR_MISS};
%
%extern int __nis_debuglevel;
%
%
#endif

#ifdef RPC_CLNT
%#include "../gen/nis_clnt.h"
#endif

program CACHEPROG {
	version CACHE_VER_1 {
		void NIS_CACHE_ADD_ENTRY(fd_result) = 1;
		void NIS_CACHE_REMOVE_ENTRY(directory_obj) = 2;
		void NIS_CACHE_READ_COLDSTART(void) = 3;
		void NIS_CACHE_REFRESH_ENTRY(string<>) = 4;
	} = 1;
} = 100301;
