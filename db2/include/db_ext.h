/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _db_ext_h_
#define _db_ext_h_
int __db_close __P((DB *, u_int32_t));
int __db_init_wrapper __P((DB *));
int __db_cprint __P((DB *));
int __db_c_destroy __P((DBC *));
int __db_sync __P((DB *, u_int32_t));
int __db_addrem_log
    __P((DB_LOG *, DB_TXN *, DB_LSN *, u_int32_t,
    u_int32_t, u_int32_t, db_pgno_t, u_int32_t,
    size_t, const DBT *, const DBT *, DB_LSN *));
int __db_addrem_print
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_addrem_read __P((void *, __db_addrem_args **));
int __db_split_log
    __P((DB_LOG *, DB_TXN *, DB_LSN *, u_int32_t,
    u_int32_t, u_int32_t, db_pgno_t, const DBT *,
    DB_LSN *));
int __db_split_print
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_split_read __P((void *, __db_split_args **));
int __db_big_log
    __P((DB_LOG *, DB_TXN *, DB_LSN *, u_int32_t,
    u_int32_t, u_int32_t, db_pgno_t, db_pgno_t,
    db_pgno_t, const DBT *, DB_LSN *, DB_LSN *,
    DB_LSN *));
int __db_big_print
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_big_read __P((void *, __db_big_args **));
int __db_ovref_log
    __P((DB_LOG *, DB_TXN *, DB_LSN *, u_int32_t,
    u_int32_t, db_pgno_t, int32_t, DB_LSN *));
int __db_ovref_print
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_ovref_read __P((void *, __db_ovref_args **));
int __db_relink_log
    __P((DB_LOG *, DB_TXN *, DB_LSN *, u_int32_t,
    u_int32_t, u_int32_t, db_pgno_t, DB_LSN *,
    db_pgno_t, DB_LSN *, db_pgno_t, DB_LSN *));
int __db_relink_print
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_relink_read __P((void *, __db_relink_args **));
int __db_addpage_log
    __P((DB_LOG *, DB_TXN *, DB_LSN *, u_int32_t,
    u_int32_t, db_pgno_t, DB_LSN *, db_pgno_t,
    DB_LSN *));
int __db_addpage_print
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_addpage_read __P((void *, __db_addpage_args **));
int __db_debug_log
    __P((DB_LOG *, DB_TXN *, DB_LSN *, u_int32_t,
    const DBT *, u_int32_t, const DBT *, const DBT *,
    u_int32_t));
int __db_debug_print
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_debug_read __P((void *, __db_debug_args **));
int __db_init_print __P((DB_ENV *));
int __db_init_recover __P((DB_ENV *));
int __db_pgin __P((db_pgno_t, size_t, void *));
int __db_pgout __P((db_pgno_t, size_t, void *));
int __db_dispatch __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_add_recovery __P((DB_ENV *,
   int (*)(DB_LOG *, DBT *, DB_LSN *, int, void *), u_int32_t));
int __db_txnlist_init __P((void *));
int __db_txnlist_add __P((void *, u_int32_t));
int __db_txnlist_find __P((void *, u_int32_t));
void __db_txnlist_end __P((void *));
void __db_txnlist_gen __P((void *, int));
void __db_txnlist_print __P((void *));
int __db_dput __P((DBC *, DBT *,
   PAGE **, db_indx_t *, int (*)(DBC *, u_int32_t, PAGE **)));
int __db_drem __P((DBC *,
   PAGE **, u_int32_t, int (*)(DBC *, PAGE *)));
int __db_dend __P((DBC *, db_pgno_t, PAGE **));
 int __db_ditem __P((DBC *, PAGE *, u_int32_t, u_int32_t));
int __db_pitem
    __P((DBC *, PAGE *, u_int32_t, u_int32_t, DBT *, DBT *));
int __db_relink __P((DBC *, u_int32_t, PAGE *, PAGE **, int));
int __db_ddup __P((DBC *, db_pgno_t, int (*)(DBC *, PAGE *)));
int __db_dsearch __P((DBC *,
    int, DBT *, db_pgno_t, db_indx_t *, PAGE **, int *));
int __db_cdelchk __P((const DB *, u_int32_t, int, int));
int __db_cgetchk __P((const DB *, DBT *, DBT *, u_int32_t, int));
int __db_cputchk __P((const DB *,
   const DBT *, DBT *, u_int32_t, int, int));
int __db_closechk __P((const DB *, u_int32_t));
int __db_delchk __P((const DB *, DBT *, u_int32_t, int));
int __db_getchk __P((const DB *, const DBT *, DBT *, u_int32_t));
int __db_joinchk __P((const DB *, u_int32_t));
int __db_putchk
   __P((const DB *, DBT *, const DBT *, u_int32_t, int, int));
int __db_statchk __P((const DB *, u_int32_t));
int __db_syncchk __P((const DB *, u_int32_t));
int __db_eopnotsup __P((const DB_ENV *));
int __db_join __P((DB *, DBC **, u_int32_t, DBC **));
int __db_goff __P((DB *, DBT *,
    u_int32_t, db_pgno_t, void **, u_int32_t *));
int __db_poff __P((DBC *, const DBT *, db_pgno_t *,
    int (*)(DBC *, u_int32_t, PAGE **)));
int __db_ovref __P((DBC *, db_pgno_t, int32_t));
int __db_doff __P((DBC *, db_pgno_t, int (*)(DBC *, PAGE *)));
int __db_moff __P((DB *, const DBT *, db_pgno_t, u_int32_t,
    int (*)(const DBT *, const DBT *), int *));
void __db_loadme __P((void));
FILE *__db_prinit __P((FILE *));
int __db_dump __P((DB *, char *, int));
int __db_prdb __P((DB *));
int __db_prbtree __P((DB *));
int __db_prhash __P((DB *));
int __db_prtree __P((DB_MPOOLFILE *, int));
int __db_prnpage __P((DB_MPOOLFILE *, db_pgno_t));
int __db_prpage __P((PAGE *, int));
int __db_isbad __P((PAGE *, int));
void __db_pr __P((u_int8_t *, u_int32_t));
int __db_prdbt __P((DBT *, int, FILE *));
void __db_prflags __P((u_int32_t, const FN *, FILE *));
int __db_addrem_recover
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_split_recover __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_big_recover __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_ovref_recover __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_relink_recover
  __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_addpage_recover
   __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_debug_recover __P((DB_LOG *, DBT *, DB_LSN *, int, void *));
int __db_ret __P((DB *,
   PAGE *, u_int32_t, DBT *, void **, u_int32_t *));
int __db_retcopy __P((DBT *,
   void *, u_int32_t, void **, u_int32_t *, void *(*)(size_t)));
#endif /* _db_ext_h_ */
