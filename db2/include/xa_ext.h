/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _xa_ext_h_
#define _xa_ext_h_
int __db_rmid_to_env __P((int rmid, DB_ENV **envp, int open_ok));
int __db_xid_to_txn __P((DB_ENV *, XID *, size_t *));
int __db_map_rmid __P((int, DB_ENV *));
int __db_unmap_rmid __P((int));
int __db_map_xid __P((DB_ENV *, XID *, size_t));
void __db_unmap_xid __P((DB_ENV *, XID *, size_t));
int __db_map_rmid_name __P((int, char *));
int __db_rmid_to_name __P((int, char **));
 void __db_unmap_rmid_name __P((int));
#endif /* _xa_ext_h_ */
