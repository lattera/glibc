/* Do not edit: automatically built by dist/distrib. */
int __db_abspath __P((const char *));
char *__db_rpath __P((const char *));
int __db_dir __P((DB_ENV *, const char *, char ***, int *));
void __db_dirf __P((DB_ENV *, char **, int));
int __db_fileid __P((DB_ENV *, const char *, int, u_int8_t *));
int __db_lseek __P((int, size_t, db_pgno_t, u_long, int));
int __db_mmap __P((int, size_t, int, int, void *));
int __db_munmap __P((void *, size_t));
int __db_oflags __P((int));
int __db_fdopen __P((const char *, int, int, int, int *));
int __db_fsync __P((int));
int __db_close __P((int));
int __db_read __P((int, void *, size_t, ssize_t *));
int __db_write __P((int, void *, size_t, ssize_t *));
int __db_sleep __P((u_long, u_long));
int __db_exists __P((const char *, int *));
int __db_stat __P((DB_ENV *, const char *, int, off_t *, off_t *));
int __db_unlink __P((const char *));
