/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _lock_ext_h_
#define _lock_ext_h_
void __lock_dump_region __P((DB_LOCKTAB *, u_int));
int __lock_is_locked
   __P((DB_LOCKTAB *, u_int32_t, DBT *, db_lockmode_t));
int __lock_getobj  __P((DB_LOCKTAB *,
    u_int32_t, const DBT *, u_int32_t type, DB_LOCKOBJ **));
int __lock_cmp __P((const DBT *, DB_LOCKOBJ *));
int __lock_locker_cmp __P((u_int32_t, DB_LOCKOBJ *));
int __lock_ohash __P((const DBT *));
u_int32_t __lock_locker_hash __P((u_int32_t));
u_int32_t __lock_lhash __P((DB_LOCKOBJ *));
#endif /* _lock_ext_h_ */
