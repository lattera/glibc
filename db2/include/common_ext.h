/* Do not edit: automatically built by dist/distrib. */
int __db_appname __P((DB_ENV *,
   APPNAME, const char *, const char *, int *, char **));
int __db_apprec __P((DB_ENV *, int));
int __db_byteorder __P((DB_ENV *, int));
#ifdef __STDC__
void __db_err __P((const DB_ENV *dbenv, const char *fmt, ...));
#else
void __db_err();
#endif
int __db_panic __P((DB *));
int __db_fchk __P((DB_ENV *, const char *, int, int));
int __db_fcchk __P((DB_ENV *, const char *, int, int, int));
int __db_cdelchk __P((const DB *, int, int, int));
int __db_cgetchk __P((const DB *, DBT *, DBT *, int, int));
int __db_cputchk __P((const DB *,
   const DBT *, DBT *, int, int, int));
int __db_delchk __P((const DB *, int, int));
int __db_getchk __P((const DB *, const DBT *, DBT *, int));
int __db_putchk __P((const DB *, DBT *, const DBT *, int, int, int));
int __db_statchk __P((const DB *, int));
int __db_syncchk __P((const DB *, int));
int __db_ferr __P((const DB_ENV *, const char *, int));
u_int32_t __db_log2 __P((u_int32_t));
int __db_rcreate __P((DB_ENV *, APPNAME,
   const char *, const char *, int, size_t, int *, void *));
int __db_ropen __P((DB_ENV *,
   APPNAME, const char *, const char *, int, int *, void *));
int __db_rclose __P((DB_ENV *, int, void *));
int __db_runlink __P((DB_ENV *,
   APPNAME, const char *, const char *, int));
int __db_rgrow __P((DB_ENV *, int, size_t));
int __db_rremap __P((DB_ENV *, void *, size_t, size_t, int, void *));
void __db_shalloc_init __P((void *, size_t));
int __db_shalloc __P((void *, size_t, size_t, void *));
void __db_shalloc_free __P((void *, void *));
size_t __db_shalloc_count __P((void *));
size_t __db_shsizeof __P((void *));
void __db_shalloc_dump __P((FILE *, void *));
int __db_tablesize __P((int));
void __db_hashinit __P((void *, int));
