#ifndef _DB_H_

#ifdef USE_LIBDB1
# include <db/db.h>
#else
# include <db2/db.h>

/* Internal interface for NSS.  */
int __nss_db_open __P((const char *,
		       DBTYPE, u_int32_t, int, DB_ENV *, DB_INFO *, DB **));
#endif

#endif
