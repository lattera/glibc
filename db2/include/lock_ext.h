/* Do not edit: automatically built by dist/distrib. */
int __lock_getobj  __P((DB_LOCKTAB *,
    u_int32_t, DBT *, u_int32_t type, DB_LOCKOBJ **));
int __lock_cmp __P((DBT *, DB_LOCKOBJ *));
int __lock_locker_cmp __P((u_int32_t, DB_LOCKOBJ *));
int __lock_ohash __P((DBT *));
u_int32_t __lock_locker_hash __P((u_int32_t));
u_int32_t __lock_lhash __P((DB_LOCKOBJ *));
