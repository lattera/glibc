#ifndef __RPCSVC_NIS_CACHE_H_
#define __RPCSVC_NIS_CACHE_H_

#include <features.h>
#include <rpc/rpc.h>
#include <rpc/types.h>
#include <rpcsvc/nis.h>

__BEGIN_DECLS

/* default cache file */
#define CACHEFILE "/var/nis/NIS_SHARED_DIRCACHE" 

/* clients have to read-lock the cache file, and SVR4 locking requires that */
/*   the file be writable, but we don't want a world-writable cache file.   */
/*   So... everyone agrees to use a different, world-writable file for the  */
/*   locking operations, but the data is in CACHEFILE.                      */
#define CACHELOCK "/usr/tmp/.NIS_DIR_CACHELOCK"

/* the file containing one trusted XDR'ed directory object.
 * This has to be present for the system to work.
 */
#define COLD_START_FILE "/var/nis/NIS_COLD_START"

enum pc_status {HIT, MISS, NEAR_MISS};

#define CACHEPROG ((u_long)100301)
#define CACHE_VER_1 ((u_long)1)

#define NIS_CACHE_ADD_ENTRY ((u_long)1)
#define NIS_CACHE_REMOVE_ENTRY ((u_long)2)
#define NIS_CACHE_READ_COLDSTART ((u_long)3)
#define NIS_CACHE_REFRESH_ENTRY ((u_long)4)

extern void *nis_cache_add_entry_1 __P ((fd_result *, CLIENT *));
extern void *nis_cache_add_entry_1_svc __P ((fd_result *, struct svc_req *));
extern void *nis_cache_remove_entry_1 __P ((directory_obj *, CLIENT *));
extern void *nis_cache_remove_entry_1_svc __P ((directory_obj *,
						struct svc_req *));
extern void *nis_cache_read_coldstart_1 __P ((void *, CLIENT *));
extern void *nis_cache_read_coldstart_1_svc __P ((void *, struct svc_req *));
extern void *nis_cache_refresh_entry_1 __P ((char **, CLIENT *));
extern void *nis_cache_refresh_entry_1_svc __P ((char **, struct svc_req *));

__END_DECLS

#endif /* !_RPCSVC_NIS_CACHE_H_ */
